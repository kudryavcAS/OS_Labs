#include "receiver.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace boost::interprocess;

int main() {

#ifdef _WIN32
	SetConsoleTitleA("Receiver");
#endif

	std::string memoryName;
	int maxMessages;
	int senderCount;

	SharedData* sharedData = nullptr;
	managed_shared_memory segment;
	named_semaphore* emptySemaphore = nullptr;
	named_semaphore* fullSemaphore = nullptr;
	named_mutex* mutex = nullptr;

	std::cout << "Enter shared memory name: ";
	std::cin >> memoryName;
	std::cout << "Enter max number of messages: ";
	inputNatural(maxMessages, MAX_MESSAGES);

	if (!createSharedMemory(memoryName, maxMessages, segment, sharedData)) {
		return 1;
	}

	if (!createSynchronizationObjects(memoryName, maxMessages, emptySemaphore, fullSemaphore, mutex)) {
		cleanupResources(emptySemaphore, fullSemaphore, mutex, sharedData);
		return 1;
	}

	std::cout << "Enter number of Sender processes: ";
	inputNatural(senderCount);

	std::string senderPath = findSenderPath();
	if (senderPath.empty()) {
		std::cout << "Sender.exe not found!\n";
		cleanupResources(emptySemaphore, fullSemaphore, mutex, sharedData);
		return 1;
	}

	if (!startSenderProcesses(senderCount, senderPath, memoryName)) {
		cleanupResources(emptySemaphore, fullSemaphore, mutex, sharedData);
		return 1;
	}

	std::this_thread::sleep_for(std::chrono::seconds(5));

	receiverLoop(sharedData, maxMessages, emptySemaphore, fullSemaphore, mutex);

	std::cout << "Close Sender windows manually\n";
	cleanupResources(emptySemaphore, fullSemaphore, mutex, sharedData);

	std::cout << "Receiver finished\n";
	return 0;
}