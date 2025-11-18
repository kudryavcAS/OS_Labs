#include "receiver.h"
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace boost::interprocess;

bool createSharedMemory(const std::string& memoryName, int maxMessages,
   managed_shared_memory& segment, SharedData*& sharedData) {
   try {
      // Удаляем сегмент, если он существует
      shared_memory_object::remove(memoryName.c_str());

      // Создаем новый сегмент разделяемой памяти
      segment = managed_shared_memory(create_only, memoryName.c_str(), sizeof(SharedData) + 1024);

      // Размещаем SharedData в разделяемой памяти
      sharedData = segment.construct<SharedData>("SharedData")();

      sharedData->readIndex = 0;
      sharedData->writeIndex = 0;
      sharedData->messageCount = 0;
      sharedData->maxMessages = maxMessages;

      std::cout << "DEBUG: Created shared memory with maxMessages = " << sharedData->maxMessages << std::endl;

      return true;
   }
   catch (const std::exception& e) {
      std::cout << "Could not create shared memory: " << e.what() << "\n";
      return false;
   }
}


bool createSynchronizationObjects(const std::string& baseName, int maxMessages,
   named_semaphore*& emptySemaphore,
   named_semaphore*& fullSemaphore,
   named_mutex*& mutex) {
   try {
      std::string emptySemName = baseName + "_empty";
      std::string fullSemName = baseName + "_full";
      std::string mutexName = baseName + "_mutex";

      // Удаляем существующие объекты
      named_semaphore::remove(emptySemName.c_str());
      named_semaphore::remove(fullSemName.c_str());
      named_mutex::remove(mutexName.c_str());

      // Создаем новые объекты
      emptySemaphore = new named_semaphore(create_only, emptySemName.c_str(), maxMessages);
      fullSemaphore = new named_semaphore(create_only, fullSemName.c_str(), 0);
      mutex = new named_mutex(create_only, mutexName.c_str());

      std::cout << "Synchronization objects created successfully\n";
      return true;
   }
   catch (const std::exception& e) {
      std::cout << "Error creating synchronization objects: " << e.what() << "\n";
      return false;
   }
}

void cleanupResources(named_semaphore* emptySemaphore,
   named_semaphore* fullSemaphore,
   named_mutex* mutex,
   SharedData* sharedData) {
   if (emptySemaphore) delete emptySemaphore;
   if (fullSemaphore) delete fullSemaphore;
   if (mutex) delete mutex;
}

std::string findSenderPath() {
   return "sender.exe";
}

bool startSenderProcesses(int senderCount, const std::string& senderPath, const std::string& memoryName) {
   for (int i = 0; i < senderCount; i++) {
      std::string command = "start \"Sender " + std::to_string(i + 1) + "\" \"" + senderPath + "\" " + memoryName;
      int result = std::system(command.c_str());
      if (result != 0) {
         std::cout << "Receiver: Failed to start Sender process " << i + 1 << "\n";
         return false;
      }
      std::cout << "Receiver: Started Sender process " << i + 1 << "\n";
   }
   return true;
}

void receiverLoop(SharedData* sharedData, int maxMessages,
   named_semaphore* emptySemaphore,
   named_semaphore* fullSemaphore,
   named_mutex* mutex) {
   bool running = true;
   while (running) {
      std::cout << "\nRECEIVER:\n";
      std::cout << "Commands:\n1 - Read message\n2 - Exit\n";
      std::cout << "Enter choice: ";

      int choice;
      inputNatural(choice, 2);

      switch (choice) {
      case 1: {
         std::cout << "Messages in buffer: " << sharedData->messageCount << "/" << maxMessages << "\n";
         if (!sharedData->messageCount) std::cout << "Waiting for message...\n";

         fullSemaphore->wait();

         Message msg; // ОБЪЯВЛЯЕМ ЗДЕСЬ, перед блоком lock
         {
            scoped_lock<named_mutex> lock(*mutex);

            msg = sharedData->messages[sharedData->readIndex]; // ПРИСВАИВАЕМ
            sharedData->readIndex = (sharedData->readIndex + 1) % maxMessages;
            sharedData->messageCount--;

            // mutex автоматически освобождается
         }

         emptySemaphore->post();

         std::cout << "Received message: '" << msg.content << "'" << "\n"; // ТЕПЕРЬ msg доступна
         break;
      }
      case 2:
         running = false;
         break;
      default:
         std::cout << "Receiver: Invalid choice\n";
         break;
      }
   }
}
