#include "receiver.h"

using namespace boost::interprocess;

bool createSharedMemory(const std::string& memoryName, int maxMessages,
	managed_shared_memory& segment, SharedData*& sharedData) {
	try {
		shared_memory_object::remove(memoryName.c_str());

		segment = managed_shared_memory(create_only, memoryName.c_str(), sizeof(SharedData) + 1024);

		sharedData = segment.construct<SharedData>("SharedData")();

		sharedData->readIndex = 0;
		sharedData->writeIndex = 0;
		sharedData->messageCount = 0;
		sharedData->maxMessages = maxMessages;

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

		named_semaphore::remove(emptySemName.c_str());
		named_semaphore::remove(fullSemName.c_str());
		named_mutex::remove(mutexName.c_str());

		emptySemaphore = new named_semaphore(create_only, emptySemName.c_str(), maxMessages);
		fullSemaphore = new named_semaphore(create_only, fullSemName.c_str(), 0);
		mutex = new named_mutex(create_only, mutexName.c_str());

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

std::string findSenderPath() {
	return "sender.exe";
}

bool startSenderProcesses(int senderCount, const std::string& senderPath, const std::string& memoryName) {
	for (int i = 0; i < senderCount; i++) {
		std::string command = "start \"Sender " + std::to_string(i + 1) + "\" \"" + senderPath + "\" " + memoryName;

		int result = std::system(command.c_str());
		if (result != 0) {
			std::cout << "Failed to start Sender process " << i + 1 << "\n";
			return false;
		}
	}
	return true;
}

bool readMessage(SharedData* sharedData, int maxMessages,
	named_semaphore* emptySemaphore,
	named_semaphore* fullSemaphore,
	named_mutex* mutex,
	Message& receivedMessage) {

	try {
		std::cout << "Messages in buffer: " << sharedData->messageCount << "/" << maxMessages << "\n";

		if (!sharedData->messageCount) {
			std::cout << "Buffer is empty! Waiting for message..." << "\n";
		}

		fullSemaphore->wait();

		{
			scoped_lock<named_mutex> lock(*mutex);

			if (sharedData->messageCount <= 0) {
				std::cout << "Error: Buffer is still empty after wait\n";
				fullSemaphore->post();
				return false;
			}

			receivedMessage = sharedData->messages[sharedData->readIndex];

			sharedData->readIndex = (sharedData->readIndex + 1) % maxMessages;
			sharedData->messageCount--;

		}

		emptySemaphore->post();

		return true;
	}
	catch (const std::exception& e) {
		std::cout << "Error reading message: " << e.what() << "\n";
		return false;
	}
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
			Message receivedMsg;
			if (readMessage(sharedData, maxMessages, emptySemaphore, fullSemaphore, mutex, receivedMsg)) {
				std::cout << "Received message: '" << receivedMsg.content << "'" << "\n";
			}
			else {
				std::cout << "Failed to read message" << "\n";
			}
			break;
		}
		case 2:
			running = false;
			break;
		default:
			std::cout << "Invalid choice\n";
			break;
		}
	}
}