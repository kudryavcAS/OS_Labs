#include <gtest/gtest.h>
#include <sstream>
#include <functional>
#include "std_thread.h"

class PrintArrayTest : public ::testing::Test {
protected:
	void SetUp() override {}

	std::string captureOutput(int* array, int size) {
		std::stringstream buffer;
		std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

		printArray(array, size);

		std::cout.rdbuf(old);
		return buffer.str();
	}
};

TEST_F(PrintArrayTest, EmptyArray) {
	int emptyArray[] = { 0, 0, 0 };
	std::string output = captureOutput(emptyArray, 3);

	EXPECT_TRUE(output.find("Array:") != std::string::npos);
	EXPECT_TRUE(output.find("0") != std::string::npos);
}

TEST_F(PrintArrayTest, PositiveNumbers) {
	int positiveArray[] = { 1, 2, 3, 4, 5 };
	std::string output = captureOutput(positiveArray, 5);

	EXPECT_TRUE(output.find("Array:") != std::string::npos);
	EXPECT_TRUE(output.find("1") != std::string::npos);
	EXPECT_TRUE(output.find("2") != std::string::npos);
	EXPECT_TRUE(output.find("3") != std::string::npos);
	EXPECT_TRUE(output.find("4") != std::string::npos);
	EXPECT_TRUE(output.find("5") != std::string::npos);
}

TEST_F(PrintArrayTest, NegativeNumbers) {
	int negativeArray[] = { -1, -2, -3 };
	std::string output = captureOutput(negativeArray, 3);

	EXPECT_TRUE(output.find("Array:") != std::string::npos);
	EXPECT_TRUE(output.find("-1") != std::string::npos);
	EXPECT_TRUE(output.find("-2") != std::string::npos);
	EXPECT_TRUE(output.find("-3") != std::string::npos);
}

TEST_F(PrintArrayTest, SingleElement) {
	int singleArray[] = { 42 };
	std::string output = captureOutput(singleArray, 1);

	EXPECT_TRUE(output.find("Array:") != std::string::npos);
	EXPECT_TRUE(output.find("42") != std::string::npos);
}

TEST_F(PrintArrayTest, LargeArray) {
	const int SIZE = 100;
	int* largeArray = new int[SIZE];
	for (int i = 0; i < SIZE; i++) {
		largeArray[i] = i * 10;
	}

	EXPECT_NO_THROW(captureOutput(largeArray, SIZE));

	delete[] largeArray;
}

TEST_F(PrintArrayTest, MixedNumbers) {
	int mixedArray[] = { 0, -5, 10, -3, 7 };
	std::string output = captureOutput(mixedArray, 5);

	EXPECT_TRUE(output.find("Array:") != std::string::npos);
	EXPECT_TRUE(output.find("-5") != std::string::npos);
	EXPECT_TRUE(output.find("10") != std::string::npos);
	EXPECT_TRUE(output.find("-3") != std::string::npos);
	EXPECT_TRUE(output.find("7") != std::string::npos);
}

TEST_F(PrintArrayTest, AllZeros) {
	int zerosArray[] = { 0, 0, 0, 0, 0 };
	std::string output = captureOutput(zerosArray, 5);

	EXPECT_TRUE(output.find("Array:") != std::string::npos);
	// Проверяем что все нули выводятся
	size_t pos = output.find("Array:");
	size_t count = 0;
	while ((pos = output.find("0", pos)) != std::string::npos) {
		++count;
		pos += 1;
	}
	EXPECT_GE(count, 5);
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
	std::streambuf* oldCinBuf;
	std::streambuf* oldCoutBuf;
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

	int result = 0;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 3);
	EXPECT_TRUE(output.find("Invalid input") != std::string::npos);
}

TEST_F(InputNaturalTest, MaxBoundaryValue_Accepted) {
	simulateInput("10");

	int result = 0;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 10);
	EXPECT_TRUE(output.empty());
}

TEST_F(InputNaturalTest, ZeroInput_Rejected) {
	simulateInput("0\n5");

	int result = 0;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 5);
	EXPECT_TRUE(output.find("Invalid input") != std::string::npos);
}

TEST_F(InputNaturalTest, NegativeInput_Rejected) {
	simulateInput("-5\n3");

	int result = 0;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 3);
	EXPECT_TRUE(output.find("Invalid input") != std::string::npos);
}

TEST_F(InputNaturalTest, MultipleInvalidThenValid) {
	simulateInput("abc\n-1\n0\n20\n4");

	int result = 0;
	std::string output = captureOutput([&]() {
		inputNatural(result, 10);
		});

	EXPECT_EQ(result, 4);

	size_t count = 0;
	size_t pos = 0;
	while ((pos = output.find("Invalid input", pos)) != std::string::npos) {
		++count;
		pos += 13;
	}
	EXPECT_GE(count, 3);
}

TEST_F(InputNaturalTest, NoMaxLimit) {
	simulateInput("1000");

	int result = 0;
	std::string output = captureOutput([&]() {
		inputNatural(result);
		});

	EXPECT_EQ(result, 1000);
	EXPECT_TRUE(output.empty());
}
