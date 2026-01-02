#include <gtest/gtest.h>
#include "receiver.h"
#include "sender.h"
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

using namespace boost::interprocess;

class SharedMemoryTest : public ::testing::Test {
protected:
	void SetUp() override {
		shared_memory_object::remove("test_memory");
		named_semaphore::remove("test_memory_empty");
		named_semaphore::remove("test_memory_full");
		named_mutex::remove("test_memory_mutex");
	}

	void TearDown() override {
		shared_memory_object::remove("test_memory");
		named_semaphore::remove("test_memory_empty");
		named_semaphore::remove("test_memory_full");
		named_mutex::remove("test_memory_mutex");
	}
};

TEST_F(SharedMemoryTest, CreateSharedMemory_Success) {
	managed_shared_memory segment;
	SharedData* sharedData = nullptr;

	bool result = createSharedMemory("test_memory", 5, segment, sharedData);

	EXPECT_TRUE(result);
	EXPECT_NE(sharedData, nullptr);
	EXPECT_EQ(sharedData->maxMessages, 5);
	EXPECT_EQ(sharedData->messageCount, 0);
	EXPECT_EQ(sharedData->readIndex, 0);
	EXPECT_EQ(sharedData->writeIndex, 0);
}

TEST_F(SharedMemoryTest, CreateSyncObjects_Success) {
	named_semaphore* emptySem = nullptr;
	named_semaphore* fullSem = nullptr;
	named_mutex* mutex = nullptr;

	bool result = createSynchronizationObjects("test_memory", 5, emptySem, fullSem, mutex);

	EXPECT_TRUE(result);
	EXPECT_NE(emptySem, nullptr);
	EXPECT_NE(fullSem, nullptr);
	EXPECT_NE(mutex, nullptr);

	delete emptySem;
	delete fullSem;
	delete mutex;
}

TEST_F(SharedMemoryTest, OpenSharedMemory_Success) {
	managed_shared_memory segment(create_only, "test_memory", sizeof(SharedData) + 1024);
	SharedData* createdData = segment.construct<SharedData>("SharedData")();
	createdData->maxMessages = 5;
	createdData->messageCount = 0;

	managed_shared_memory segment2;
	SharedData* openedData = nullptr;
	bool result = openSharedMemory("test_memory", segment2, openedData);

	EXPECT_TRUE(result);
	EXPECT_NE(openedData, nullptr);
	EXPECT_EQ(openedData->maxMessages, 5);
}

TEST_F(SharedMemoryTest, OpenSharedMemory_NotFound) {
	managed_shared_memory segment;
	SharedData* sharedData = nullptr;
	bool result = openSharedMemory("non_existent_memory", segment, sharedData);

	EXPECT_FALSE(result);
	EXPECT_EQ(sharedData, nullptr);
}

class MessageTest : public ::testing::Test {
protected:
	MessageTest() : segment(nullptr), sharedData(nullptr) {}
	void SetUp() override {
		shared_memory_object::remove("msg_test");
		segment = new managed_shared_memory(create_only, "msg_test", sizeof(SharedData) + 1024);
		sharedData = segment->construct<SharedData>("SharedData")();
		sharedData->maxMessages = 3;
		sharedData->messageCount = 0;
		sharedData->readIndex = 0;
		sharedData->writeIndex = 0;
	}

	void TearDown() override {
		delete segment;
		shared_memory_object::remove("msg_test");
	}

	managed_shared_memory* segment;
	SharedData* sharedData;
};

TEST_F(MessageTest, SendMessage_Success) {
	named_semaphore emptySem(create_only, "msg_test_empty", 3);
	named_semaphore fullSem(create_only, "msg_test_full", 0);
	named_mutex mutex(create_only, "msg_test_mutex");

	{
		scoped_lock<named_mutex> lock(mutex);
		Message msg;
		strcpy_s(msg.content, sizeof(msg.content), "Test");
		sharedData->messages[sharedData->writeIndex] = msg;
		sharedData->writeIndex = (sharedData->writeIndex + 1) % 3;
		sharedData->messageCount++;
	}

	EXPECT_EQ(sharedData->messageCount, 1);
	EXPECT_EQ(std::string(sharedData->messages[0].content), "Test");
}

TEST_F(MessageTest, ReadMessage_Success) {
	{
		strcpy_s(sharedData->messages[0].content, sizeof(sharedData->messages[0].content), "Hello");
		sharedData->messageCount = 1;
	}

	Message readMsg;
	{
		named_mutex mutex(open_only, "msg_test_mutex");
		scoped_lock<named_mutex> lock(mutex);
		readMsg = sharedData->messages[sharedData->readIndex];
		sharedData->readIndex = (sharedData->readIndex + 1) % 3;
		sharedData->messageCount--;
	}

	EXPECT_EQ(sharedData->messageCount, 0);
	EXPECT_EQ(std::string(readMsg.content), "Hello");
}

TEST_F(MessageTest, CircularBuffer_WrapAround) {
	sharedData->maxMessages = 3;

	for (int i = 0; i < 3; i++) {
		strcpy_s(sharedData->messages[i].content, sizeof(sharedData->messages[i].content),
			("Msg" + std::to_string(i)).c_str());
	}
	sharedData->messageCount = 3;
	sharedData->writeIndex = 0;

	strcpy_s(sharedData->messages[0].content, sizeof(sharedData->messages[0].content), "NewMsg");
	sharedData->writeIndex = 1;

	EXPECT_EQ(sharedData->writeIndex, 1);
}

class IntegrationTest : public ::testing::Test {
protected:
	void SetUp() override {
		shared_memory_object::remove("integration_test");
		named_semaphore::remove("integration_test_empty");
		named_semaphore::remove("integration_test_full");
		named_mutex::remove("integration_test_mutex");
	}

	void TearDown() override {
		shared_memory_object::remove("integration_test");
		named_semaphore::remove("integration_test_empty");
		named_semaphore::remove("integration_test_full");
		named_mutex::remove("integration_test_mutex");
	}
};

TEST_F(IntegrationTest, SendAndReceive_Integration) {
	managed_shared_memory segment(create_only, "integration_test", sizeof(SharedData) + 1024);
	SharedData* sharedData = segment.construct<SharedData>("SharedData")();
	sharedData->maxMessages = 2;
	sharedData->messageCount = 0;

	named_semaphore emptySem(create_only, "integration_test_empty", 2);
	named_semaphore fullSem(create_only, "integration_test_full", 0);
	named_mutex mutex(create_only, "integration_test_mutex");

	emptySem.wait();
	{
		scoped_lock<named_mutex> lock(mutex);
		Message msg;
		strcpy_s(msg.content, sizeof(msg.content), "Integration Test");
		sharedData->messages[sharedData->writeIndex] = msg;
		sharedData->writeIndex = (sharedData->writeIndex + 1) % 2;
		sharedData->messageCount++;
	}
	fullSem.post();

	EXPECT_EQ(sharedData->messageCount, 1);

	fullSem.wait();
	Message receivedMsg;
	{
		scoped_lock<named_mutex> lock(mutex);
		receivedMsg = sharedData->messages[sharedData->readIndex];
		sharedData->readIndex = (sharedData->readIndex + 1) % 2;
		sharedData->messageCount--;
	}
	emptySem.post();

	EXPECT_EQ(sharedData->messageCount, 0);
	EXPECT_EQ(std::string(receivedMsg.content), "Integration Test");
}

TEST(EdgeCasesTest, MessageLengthBoundaries) {
	Message msg;

	std::string maxLengthMsg(MAX_MESSAGE_LENGTH, 'A');
	strcpy_s(msg.content, sizeof(msg.content), maxLengthMsg.c_str());

	EXPECT_EQ(strlen(msg.content), MAX_MESSAGE_LENGTH);

	std::string tooLongMsg(MAX_MESSAGE_LENGTH + 10, 'B');
	strcpy_s(msg.content, sizeof(msg.content), tooLongMsg.c_str());

	EXPECT_LE(strlen(msg.content), MAX_MESSAGE_LENGTH);
}

TEST_F(IntegrationTest, ConcurrentAccess_NoDataRace) {
	managed_shared_memory segment(create_only, "integration_test", sizeof(SharedData) + 1024);
	SharedData* sharedData = segment.construct<SharedData>("SharedData")();
	sharedData->maxMessages = 10;

	named_semaphore emptySem(create_only, "integration_test_empty", 10);
	named_semaphore fullSem(create_only, "integration_test_full", 0);
	named_mutex mutex(create_only, "integration_test_mutex");

	for (int i = 0; i < 10; i++) {
		emptySem.wait();
		{
			scoped_lock<named_mutex> lock(mutex);
			Message msg;
			strcpy_s(msg.content, sizeof(msg.content), ("Msg" + std::to_string(i)).c_str());
			sharedData->messages[sharedData->writeIndex] = msg;
			sharedData->writeIndex = (sharedData->writeIndex + 1) % 10;
			sharedData->messageCount++;
		}
		fullSem.post();
	}

	EXPECT_EQ(sharedData->messageCount, 10);

	for (int i = 0; i < 10; i++) {
		fullSem.wait();
		{
			scoped_lock<named_mutex> lock(mutex);
			sharedData->readIndex = (sharedData->readIndex + 1) % 10;
			sharedData->messageCount--;
		}
		emptySem.post();
	}

	EXPECT_EQ(sharedData->messageCount, 0);
}

class InputNaturalTest : public ::testing::Test {
protected:
	void SetUp() override {
		oldCinBuf = std::cin.rdbuf();
		oldCoutBuf = std::cout.rdbuf();
	}

	void TearDown() override {
		std::cin.rdbuf(oldCinBuf);
		std::cout.rdbuf(oldCoutBuf);
	}

	void simulateInput(const std::string& input) {
		inputStream.str(input);
		inputStream.clear();
		std::cin.rdbuf(inputStream.rdbuf());
	}

	std::string captureOutput(std::function<void()> testFunction) {
		std::stringstream outputStream;
		std::cout.rdbuf(outputStream.rdbuf());

		testFunction();

		std::cout.rdbuf(oldCoutBuf);
		return outputStream.str();
	}

	std::stringstream inputStream;
	std::streambuf* oldCinBuf = nullptr;
	std::streambuf* oldCoutBuf = nullptr;
};

TEST_F(InputNaturalTest, ValidInput_ReturnsCorrectValue) {
	simulateInput("5");

	int result = 0;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 5);
	EXPECT_TRUE(output.empty());
}

TEST_F(InputNaturalTest, StringThenValidInput_HandlesError) {
	simulateInput("abc\n7");

	int result = 0;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 7);
	EXPECT_TRUE(output.find("Invalid input") != std::string::npos);
}

TEST_F(InputNaturalTest, OutOfRangeThenValid_HandlesError) {
	simulateInput("15\n3");

	int result;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 3);
	EXPECT_TRUE(output.find("Invalid input") != std::string::npos);
}

TEST_F(InputNaturalTest, MaxBoundaryValue_Accepted) {
	simulateInput("10");

	int result;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 10);
	EXPECT_TRUE(output.empty());
}

TEST(FindSenderPathTest, ReturnsCorrectPath) {
	EXPECT_EQ("sender.exe", findSenderPath());
}