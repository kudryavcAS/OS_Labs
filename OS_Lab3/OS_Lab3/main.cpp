#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <memory>

int arraySize = 0;
std::unique_ptr<int[]> array;

std::mutex arrayMutex;
std::mutex consoleMutex;

std::atomic<bool> threadStartFlag{ false };
std::unique_ptr<std::atomic<bool>[]> threadContinueFlags;
std::unique_ptr<std::atomic<bool>[]> threadStopFlags;
std::unique_ptr<std::atomic<bool>[]> threadCannotContinueFlags;
std::unique_ptr<std::atomic<bool>[]> threadTerminated;

void markerThread(int threadNumber) {
	int markedCount = 0, index = threadNumber - 1;
	std::random_device rd;
	std::mt19937 gen(rd());

	while (!threadStartFlag) {
		std::this_thread::yield();
	}

	while (true) {
		unsigned int randomValue = gen();
		int i = randomValue % arraySize;

		bool needToPrint = false;
		int printMarkedCount = 0;
		int printIndex = 0;

		{
			std::lock_guard<std::mutex> lock(arrayMutex);
			if (array[i] == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				array[i] = threadNumber;
				markedCount++;
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
			else {
				needToPrint = true;
				printMarkedCount = markedCount;
				printIndex = i;

				threadCannotContinueFlags[index] = true;
			}
		}
		if (needToPrint) {
			std::lock_guard<std::mutex> consoleLock(consoleMutex);
			std::cout << "Number of thread: " << threadNumber
				<< ". Count of marked elements: " << markedCount
				<< ". Index of unmarked elements: " << i << "\n";
		}
		// Если не можем продолжать, ждем команды
		if (threadCannotContinueFlags[index]) {
			// Ждем либо continue, либо stop
			while (threadCannotContinueFlags[index] && !threadStopFlags[index]) {
				std::this_thread::yield();
			}

			if (threadStopFlags[index]) {
				// Очищаем массив
				std::lock_guard<std::mutex> lock(arrayMutex);
				for (int j = 0; j < arraySize; j++) {
					if (array[j] == threadNumber) {
						array[j] = 0;
					}
				}
				{
					std::lock_guard<std::mutex> consoleLock(consoleMutex);
					std::cout << "Thread " << threadNumber << " terminated.\n";
				}
				threadTerminated[index] = true;
				return;
			}

			// Сбрасываем флаг и продолжаем работу
			threadCannotContinueFlags[index] = false;
		}
	}
}

void inputNatural(int& integer, int max = INT_MAX) {
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

void printArray(int* array, int arraySize) {
	std::lock_guard<std::mutex> lock(arrayMutex);
	std::lock_guard<std::mutex> consoleLock(consoleMutex);

	std::cout << "Array: ";
	for (int i = 0; i < arraySize; i++) {
		std::cout << array[i] << "\t";
	}
	std::cout << "\n";
}

int main() {
	int count;

	std::cout << "Enter the array size:\n";
	inputNatural(arraySize);

	array = std::make_unique<int[]>(arraySize);
	for (int i = 0; i < arraySize; i++) {
		array[i] = 0;
	}

	{
		std::lock_guard<std::mutex> consoleLock(consoleMutex);
		std::cout << "Enter the number of marker threads:\n";
	}
	inputNatural(count);

	// Инициализация массивов для управления потоками
	threadContinueFlags = std::make_unique<std::atomic<bool>[]>(count);
	threadStopFlags = std::make_unique<std::atomic<bool>[]>(count);
	threadCannotContinueFlags = std::make_unique<std::atomic<bool>[]>(count);
	threadTerminated = std::make_unique<std::atomic<bool>[]>(count);

	// Инициализация атомарных флагов
	for (int i = 0; i < count; i++) {
		threadContinueFlags[i] = false;
		threadStopFlags[i] = false;
		threadCannotContinueFlags[i] = false;
		threadTerminated[i] = false;
	}

	// Создание потоков
	std::vector<std::thread> threads;
	for (int i = 0; i < count; i++) {
		threads.emplace_back(markerThread, i + 1);
	}

	// Запускаем все потоки
	threadStartFlag = true;

	int activeThreads = count;

	while (activeThreads > 0) {
		// Ждем, пока какой-либо поток не сможет продолжить
		bool anyThreadWaiting = false;
		do {
			anyThreadWaiting = false;
			for (int i = 0; i < count; i++) {
				if (!threadTerminated[i] && threadCannotContinueFlags[i]) {
					anyThreadWaiting = true;
					break;
				}
			}
			if (!anyThreadWaiting) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		} while (!anyThreadWaiting && activeThreads > 0);

		if (activeThreads == 0) break;

		printArray(array.get(), arraySize);

		int threadToTerminate;
		{
			std::lock_guard<std::mutex> consoleLock(consoleMutex);
			std::cout << "Enter thread number to terminate (1-" << count << "): ";
		}
		inputNatural(threadToTerminate, count);

		int index = threadToTerminate - 1;

		// Устанавливаем флаг остановки
		threadStopFlags[index] = true;

		// Ждем завершения потока
		if (threads[index].joinable()) {
			threads[index].join();
		}

		printArray(array.get(), arraySize);

		// Разрешаем остальным потокам продолжить
		for (int i = 0; i < count; i++) {
			if (!threadTerminated[i]) {
				threadCannotContinueFlags[i] = false;
			}
		}
		activeThreads--;
	}

	std::lock_guard<std::mutex> consoleLock(consoleMutex);
	std::cout << "All threads completed. Program finished." << std::endl;
	return 0;
}