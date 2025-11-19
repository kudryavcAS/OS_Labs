#include "std_thread.h"

std::mutex arrayMutex;
std::mutex consoleMutex;

void inputNatural(int& integer, int max) {
	while (true) {
		std::cin >> integer;
		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			std::cout << "Invalid input. Enter an integer 0 < " << max << std::endl;
			continue;
		}
		if (integer <= 0 || integer > max) {
			std::cout << "Invalid input. Enter an integer 0 < " << max << std::endl;
			continue;
		}
		break;
	}
}

void printArray(int* array, int arraySize) {
	std::lock_guard<std::mutex> lock(arrayMutex);
	std::lock_guard<std::mutex> consoleLock(consoleMutex);

	std::cout << "Array: ";
	for (int i = 0; i < arraySize; i++) {
		std::cout << array[i] << "\t";
	}
	std::cout << std::endl;
}
