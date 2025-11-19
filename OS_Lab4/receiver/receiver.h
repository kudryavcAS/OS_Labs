#ifndef RECEIVER_H
#define RECEIVER_H

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include <thread>
#include <chrono>
#include <cstdlib>


const int MAX_MESSAGE_LENGTH = 20;
const int MAX_MESSAGES = 100;

struct Message {
	char content[MAX_MESSAGE_LENGTH + 1];
};

struct SharedData {
	Message messages[MAX_MESSAGES];
	int readIndex;
	int writeIndex;
	int messageCount;
	int maxMessages;
};

inline void inputNatural(int& integer, int max = INT_MAX) {
	while (true) {
		std::cin >> integer;
		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			std::cout << "Invalid input. Enter an integer 0 < " << max << "\n";
			continue;
		}
		if (integer <= 0 || integer > max) {
			std::cout << "Invalid input. Enter an integer 0 < " << max << "\n";
			continue;
		}
		break;
	}
}

bool createSharedMemory(const std::string& memoryName, int maxMessages, boost::interprocess::managed_shared_memory& segment, SharedData*& sharedData);
bool createSynchronizationObjects(const std::string& baseName, int maxMessages,
	boost::interprocess::named_semaphore*& emptySemaphore,
	boost::interprocess::named_semaphore*& fullSemaphore,
	boost::interprocess::named_mutex*& mutex);
void cleanupResources(boost::interprocess::named_semaphore* emptySemaphore,
	boost::interprocess::named_semaphore* fullSemaphore,
	boost::interprocess::named_mutex* mutex,
	SharedData* sharedData);
std::string findSenderPath();
bool startSenderProcesses(int senderCount, const std::string& senderPath, const std::string& memoryName);
void receiverLoop(SharedData* sharedData, int maxMessages,
	boost::interprocess::named_semaphore* emptySemaphore,
	boost::interprocess::named_semaphore* fullSemaphore,
	boost::interprocess::named_mutex* mutex);

#endif