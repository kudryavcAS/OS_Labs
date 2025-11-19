#ifndef SENDER_H
#define SENDER_H

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <string>
#include <cstring>
#include "receiver.h"

bool openSharedMemory(const std::string& memoryName, boost::interprocess::managed_shared_memory& segment, SharedData*& sharedData);
bool openSynchronizationObjects(const std::string& baseName,
	boost::interprocess::named_semaphore*& emptySemaphore,
	boost::interprocess::named_semaphore*& fullSemaphore,
	boost::interprocess::named_mutex*& mutex);
void senderLoop(SharedData* sharedData,
	boost::interprocess::named_semaphore* emptySemaphore,
	boost::interprocess::named_semaphore* fullSemaphore,
	boost::interprocess::named_mutex* mutex);
void cleanupSenderResources(boost::interprocess::named_semaphore* emptySemaphore,
	boost::interprocess::named_semaphore* fullSemaphore,
	boost::interprocess::named_mutex* mutex,
	SharedData* sharedData);
bool sendMessage(SharedData* sharedData,
	boost::interprocess::named_semaphore* emptySemaphore,
	boost::interprocess::named_semaphore* fullSemaphore,
	boost::interprocess::named_mutex* mutex);

#endif