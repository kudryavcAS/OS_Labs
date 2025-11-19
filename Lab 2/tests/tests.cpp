#include <gtest/gtest.h>
#include "ArrayFunctions.h"
#include <tuple>

auto runMinMaxTest(const std::vector<int>& arr) {
	ArrayData data(&arr, 0, 0, 0);
	DWORD result = searchMinMaxElement(&data);
	return std::tuple{ result, data.minElement, data.maxElement };
}

auto runAverageTest(const std::vector<int>& arr) {
	ArrayData data(&arr, 0, 0, 0);
	DWORD result = searchAverage(&data);
	return std::tuple{ result, data.average };
}
TEST(MinMaxSearch, SimpleArrayCase) {
	auto [result, min, max] = runMinMaxTest({ 5, 2, 8, 1, 9, 3 });

	EXPECT_EQ(result, 0);
	EXPECT_EQ(min, 1);
	EXPECT_EQ(max, 9);
}

TEST(MinMaxSearch, SimilarNumbersCase) {
	auto [result, min, max] = runMinMaxTest({ 2, 2, 2, 2, 2, 2 });

	EXPECT_EQ(result, 0);
	EXPECT_EQ(min, 2);
	EXPECT_EQ(max, 2);
}

TEST(MinMaxSearch, NegativeNumbersCase) {
	auto [result, min, max] = runMinMaxTest({ -2, -3, -4, -5, -10, -1900 });

	EXPECT_EQ(result, 0);
	EXPECT_EQ(min, -1900);
	EXPECT_EQ(max, -2);
}

TEST(MinMaxSearch, SingleNumberCase) {
	auto [result, min, max] = runMinMaxTest({ 5 });

	EXPECT_EQ(result, 0);
	EXPECT_EQ(min, 5);
	EXPECT_EQ(max, 5);
}

TEST(MinMaxSearch, EmptyArrayCase) {
	auto [result, min, max] = runMinMaxTest({});

	EXPECT_EQ(result, 0);
}

TEST(AverageSearh, EmptyArrayCase) {
	auto [result, average] = runAverageTest({});

	EXPECT_EQ(result, 0);
}

TEST(AverageSearh, SimpleArrayCase) {
	std::vector<int> arr = { 5, 2, 8, 1, 9, 3 };
	ArrayData data(&arr, 0, 0, 0);
	auto [result, average] = runAverageTest({ 5, 2, 8, 1, 9, 3 });

	EXPECT_EQ(result, 0);
	EXPECT_EQ(average, 5);
}

TEST(AverageSearh, SimilarNumbersCase) {
	auto [result, average] = runAverageTest({ 5, 5, 5, 5, 5, 5 });

	EXPECT_EQ(result, 0);
	EXPECT_EQ(average, 5);
}

TEST(AverageSearh, NegativeNumbersCase) {
	std::vector<int> arr = { -2, -3, -4, -5, -10, -13 };
	ArrayData data(&arr, 0, 0, 0);
	auto [result, average] = runAverageTest({ -2, -3, -4, -5, -10, -13 });

	EXPECT_EQ(result, 0);
	EXPECT_EQ(average, -6);
}

TEST(AverageSearh, SingleNumberCase) {
	auto [result, average] = runAverageTest({ 6 });

	EXPECT_EQ(result, 0); 
	EXPECT_EQ(average, 6);
}

TEST(AverageSearh, HugeNumbers) {
	auto [result, average] = runAverageTest({ 1000000000, 1500000000, 2000000000 });

	EXPECT_EQ(result, 0);
	EXPECT_EQ(average, 1500000000);
}
