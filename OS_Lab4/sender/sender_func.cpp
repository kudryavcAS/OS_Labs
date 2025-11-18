#include "sender.h"

using namespace boost::interprocess;

bool openSharedMemory(const std::string& memoryName, managed_shared_memory& segment, SharedData*& sharedData) {
   try {
      segment = managed_shared_memory(open_only, memoryName.c_str());
      sharedData = segment.find<SharedData>("SharedData").first;

      if (sharedData == nullptr) {
         std::cout << "Could not find SharedData in shared memory\n";
         return false;
      }

      std::cout << "DEBUG: Successfully opened shared memory\n";
      std::cout << "DEBUG: maxMessages = " << sharedData->maxMessages << std::endl;
      std::cout << "DEBUG: messageCount = " << sharedData->messageCount << std::endl;
      std::cout << "DEBUG: readIndex = " << sharedData->readIndex << std::endl;
      std::cout << "DEBUG: writeIndex = " << sharedData->writeIndex << std::endl;

      return true;
   }
   catch (const std::exception& e) {
      std::cout << "Could not open shared memory: " << e.what() << "\n";
      std::cout << "Make sure Receiver is running first!" << "\n";
      return false;
   }
}

bool openSynchronizationObjects(const std::string& baseName,
   named_semaphore*& emptySemaphore,
   named_semaphore*& fullSemaphore,
   named_mutex*& mutex) {
   try {
      std::string emptySemName = baseName + "_empty";
      std::string fullSemName = baseName + "_full";
      std::string mutexName = baseName + "_mutex";

      std::cout << "Opening synchronization objects..." << "\n";
      std::cout << "  Empty semaphore: " << emptySemName << "\n";
      std::cout << "  Full semaphore: " << fullSemName << "\n";
      std::cout << "  Mutex: " << mutexName << "\n";

      emptySemaphore = new named_semaphore(open_only, emptySemName.c_str());
      fullSemaphore = new named_semaphore(open_only, fullSemName.c_str());
      mutex = new named_mutex(open_only, mutexName.c_str());

      std::cout << "Successfully opened all synchronization objects!\n";
      return true;
   }
   catch (const std::exception& e) {
      std::cout << "Could not open synchronization objects: " << e.what() << "\n";
      return false;
   }
}

void cleanupSenderResources(named_semaphore* emptySemaphore,
   named_semaphore* fullSemaphore,
   named_mutex* mutex,
   SharedData* sharedData) {
   if (emptySemaphore) {
      delete emptySemaphore;
      emptySemaphore = nullptr;
   }
   if (fullSemaphore) {
      delete fullSemaphore;
      fullSemaphore = nullptr;
   }
   if (mutex) {
      delete mutex;
      mutex = nullptr;
   }
}

bool sendMessage(SharedData* sharedData,
   named_semaphore* emptySemaphore,
   named_semaphore* fullSemaphore,
   named_mutex* mutex) {

   std::cout << "Current messages in buffer: " << sharedData->messageCount << "/" << sharedData->maxMessages << "\n";

   if (sharedData->messageCount >= sharedData->maxMessages) {
      std::cout << "Buffer is full! Waiting for space..." << "\n";
   }

   try {
      // Ждем свободного места
      emptySemaphore->wait();

      {
         // Блокируем мьютекс
         scoped_lock<named_mutex> lock(*mutex);

         // Проверяем, есть ли место (двойная проверка)
         if (sharedData->messageCount >= sharedData->maxMessages) {
            std::cout << "Error: Buffer is still full after wait\n";
            emptySemaphore->post(); // Возвращаем семафор
            return false;
         }

         // Получаем сообщение
         std::string message;
         std::cout << "Enter message (max " << MAX_MESSAGE_LENGTH << " chars): ";
         std::cin >> message;

         if (message.length() > MAX_MESSAGE_LENGTH) {
            std::cout << "Message too long" << "\n";
            emptySemaphore->post(); // Возвращаем семафор
            return false;
         }

         // Копируем сообщение
         Message msg;
         strcpy_s(msg.content, sizeof(msg.content), message.c_str());

         // Записываем в буфер
         sharedData->messages[sharedData->writeIndex] = msg;
         sharedData->writeIndex = (sharedData->writeIndex + 1) % sharedData->maxMessages;
         sharedData->messageCount++;

         std::cout << "Message written to position: " << sharedData->writeIndex << std::endl;
      }

      // Уведомляем получателя о новом сообщении
      fullSemaphore->post();

      std::cout << "Message sent successfully!\n";
      return true;
   }
   catch (const std::exception& e) {
      std::cout << "Error sending message: " << e.what() << "\n";
      return false;
   }
}

void senderLoop(SharedData* sharedData,
   named_semaphore* emptySemaphore,
   named_semaphore* fullSemaphore,
   named_mutex* mutex) {
   bool running = true;
   while (running) {
      std::cout << "\n=== SENDER ===" << "\n";
      std::cout << "Commands:\n1 - Send message\n2 - Exit" << "\n";
      std::cout << "Enter choice: ";

      int choice;
      inputNatural(choice, 2);

      switch (choice) {
      case 1:
         sendMessage(sharedData, emptySemaphore, fullSemaphore, mutex);
         break;
      case 2:
         running = false;
         break;
      default:
         std::cout << "Invalid choice" << "\n";
         break;
      }
   }
}