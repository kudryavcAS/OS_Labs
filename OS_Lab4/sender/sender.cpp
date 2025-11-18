#include "sender.h"

using namespace boost::interprocess;

int main(int argc, char* argv[]) {
   if (argc < 2) {
      std::cout << "Memory name not provided" << "\n";
      std::cout << "Usage: sender <memory_name>" << "\n";
      return 1;
   }

   std::string memoryName = argv[1];
   std::cout << "SENDER:\n" << "\n";
   std::cout << "Ready to work with shared memory: " << memoryName << "\n";

   SharedData* sharedData = nullptr;
   managed_shared_memory segment;
   named_semaphore* emptySemaphore = nullptr;
   named_semaphore* fullSemaphore = nullptr;
   named_mutex* mutex = nullptr;

   if (!openSharedMemory(memoryName, segment, sharedData)) {
      return 1;
   }

   if (!openSynchronizationObjects(memoryName, emptySemaphore, fullSemaphore, mutex)) {
      cleanupSenderResources(emptySemaphore, fullSemaphore, mutex, sharedData);
      return 1;
   }

   std::cout << "Maximum messages: " << sharedData->maxMessages << "\n";
   std::cout << "Current messages: " << sharedData->messageCount << "\n";

   senderLoop(sharedData, emptySemaphore, fullSemaphore, mutex);

   cleanupSenderResources(emptySemaphore, fullSemaphore, mutex, sharedData);
   std::cout << "Finished" << "\n";

   return 0;
}