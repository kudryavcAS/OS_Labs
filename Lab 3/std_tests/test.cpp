#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <iostream>
#include <functional>
#include <string>
#include <climits>
#include "std_thread.h"

struct PrintArrayFixture {
	std::string captureOutput(int* array, int size) {
		std::stringstream buffer;
		std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

		printArray(array, size);

		std::cout.rdbuf(old);
		return buffer.str();
	}
};

TEST_CASE_METHOD(PrintArrayFixture, "PrintArray logic", "[output]") {

	SECTION("Empty Array") {
		int emptyArray[] = { 0, 0, 0 };
		std::string output = captureOutput(emptyArray, 3);

		CHECK(output.find("Array:") != std::string::npos);
		CHECK(output.find("0") != std::string::npos);
	}

	SECTION("Positive Numbers") {
		int positiveArray[] = { 1, 2, 3, 4, 5 };
		std::string output = captureOutput(positiveArray, 5);

		CHECK(output.find("Array:") != std::string::npos);
		CHECK(output.find("1") != std::string::npos);
		CHECK(output.find("2") != std::string::npos);
		CHECK(output.find("3") != std::string::npos);
		CHECK(output.find("4") != std::string::npos);
		CHECK(output.find("5") != std::string::npos);
	}

	SECTION("Negative Numbers") {
		int negativeArray[] = { -1, -2, -3 };
		std::string output = captureOutput(negativeArray, 3);

		CHECK(output.find("Array:") != std::string::npos);
		CHECK(output.find("-1") != std::string::npos);
		CHECK(output.find("-2") != std::string::npos);
		CHECK(output.find("-3") != std::string::npos);
	}

	SECTION("Single Element") {
		int singleArray[] = { 42 };
		std::string output = captureOutput(singleArray, 1);

		CHECK(output.find("Array:") != std::string::npos);
		CHECK(output.find("42") != std::string::npos);
	}

	SECTION("Large Array") {
		const int SIZE = 100;
		int* largeArray = new int[SIZE];
		for (int i = 0; i < SIZE; i++) {
			largeArray[i] = i * 10;
		}

		CHECK_NOTHROW(captureOutput(largeArray, SIZE));

		delete[] largeArray;
	}

	SECTION("Mixed Numbers") {
		int mixedArray[] = { 0, -5, 10, -3, 7 };
		std::string output = captureOutput(mixedArray, 5);

		CHECK(output.find("Array:") != std::string::npos);
		CHECK(output.find("-5") != std::string::npos);
		CHECK(output.find("10") != std::string::npos);
		CHECK(output.find("-3") != std::string::npos);
		CHECK(output.find("7") != std::string::npos);
	}

	SECTION("All Zeros") {
		int zerosArray[] = { 0, 0, 0, 0, 0 };
		std::string output = captureOutput(zerosArray, 5);

		CHECK(output.find("Array:") != std::string::npos);

		size_t pos = output.find("Array:");
		size_t count = 0;
		while ((pos = output.find("0", pos)) != std::string::npos) {
			++count;
			pos += 1;
		}
		CHECK(count >= 5);
	}
}

struct InputNaturalFixture {
	std::stringstream inputStream;
	std::streambuf* oldCinBuf;
	std::streambuf* oldCoutBuf;

	InputNaturalFixture() {
		oldCinBuf = std::cin.rdbuf();
		oldCoutBuf = std::cout.rdbuf();
	}

	~InputNaturalFixture() {
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
};

TEST_CASE_METHOD(InputNaturalFixture, "InputNatural logic", "[input]") {

	SECTION("ValidInput_ReturnsCorrectValue") {
		simulateInput("5");

		int result = 0;
		std::string output = captureOutput([&]() {
			inputNatural(result, 10);
			});

		CHECK(result == 5);
		CHECK(output.empty());
	}

	SECTION("StringThenValidInput_HandlesError") {
		simulateInput("abc\n7");

		int result = 0;
		std::string output = captureOutput([&]() {
			inputNatural(result, 10);
			});

		CHECK(result == 7);
		CHECK(output.find("Invalid input") != std::string::npos);
	}

	SECTION("OutOfRangeThenValid_HandlesError") {
		simulateInput("15\n3");

		int result = 0;
		std::string output = captureOutput([&]() {
			inputNatural(result, 10);
			});

		CHECK(result == 3);
		CHECK(output.find("Invalid input") != std::string::npos);
	}

	SECTION("MaxBoundaryValue_Accepted") {
		simulateInput("10");

		int result = 0;
		std::string output = captureOutput([&]() {
			inputNatural(result, 10);
			});

		CHECK(result == 10);
		CHECK(output.empty());
	}

	SECTION("ZeroInput_Rejected") {
		simulateInput("0\n5");

		int result = 0;
		std::string output = captureOutput([&]() {
			inputNatural(result, 10);
			});

		CHECK(result == 5);
		CHECK(output.find("Invalid input") != std::string::npos);
	}

	SECTION("NegativeInput_Rejected") {
		simulateInput("-5\n3");

		int result = 0;
		std::string output = captureOutput([&]() {
			inputNatural(result, 10);
			});

		CHECK(result == 3);
		CHECK(output.find("Invalid input") != std::string::npos);
	}

	SECTION("MultipleInvalidThenValid") {
		simulateInput("abc\n-1\n0\n20\n4");

		int result = 0;
		std::string output = captureOutput([&]() {
			inputNatural(result, 10);
			});

		CHECK(result == 4);

		size_t count = 0;
		size_t pos = 0;
		while ((pos = output.find("Invalid input", pos)) != std::string::npos) {
			++count;
			pos += 13;
		}
		CHECK(count >= 3);
	}

	SECTION("NoMaxLimit") {
		simulateInput("1000");

		int result = 0;
		std::string output = captureOutput([&]() {
			inputNatural(result);
			});

		CHECK(result == 1000);
		CHECK(output.empty());
	}
}