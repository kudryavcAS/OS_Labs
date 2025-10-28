#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <memory>
#include <condition_variable>

// Глобальные переменные
int arraySize = 0;
std::unique_ptr<int[]> array;

// Примитивы синхронизации
std::mutex arrayMutex;
std::mutex consoleMutex;
std::atomic<bool> threadStartFlag{ false };

// Структура для состояния потока
struct ThreadState {
	std::condition_variable continueCV;
	std::condition_variable stopCV;
	std::condition_variable cannotContinueCV;
	std::atomic<bool> cannotContinue{ false };
	std::atomic<bool> terminated{ false };
};

// Управление потоками
std::vector<std::unique_ptr<ThreadState>> threadStates;
std::vector<std::thread> markerThreads;

void markerThread(int threadNumber) {
	int markedCount = 0;
	int index = threadNumber - 1;
	auto& state = *threadStates[index];

	// Инициализация генератора случайных чисел (по условию)
	srand(threadNumber);

	// Ждем старта от main
	while (!threadStartFlag) {
		std::this_thread::yield();
	}

	while (true) {
		// Генерация случайного числа (по условию)
		int randomValue = rand();
		int i = randomValue % arraySize;

		{
			std::lock_guard<std::mutex> lock(arrayMutex);
			if (array[i] == 0) {
				// Спим 5 мс (по условию)
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				array[i] = threadNumber;
				markedCount++;
				// Спим еще 5 мс (по условию)
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
			else {
				// Вывод информации (по условию)
				std::lock_guard<std::mutex> consoleLock(consoleMutex);
				std::cout << "Number of thread: " << threadNumber
					<< ". Count of marked elements: " << markedCount
					<< ". Index of unmarked elements: " << i << std::endl;

				// Сигнализируем о невозможности продолжения
				state.cannotContinue = true;
				state.cannotContinueCV.notify_one();
			}
		}

		// Если не можем продолжать, ждем сигнал от main
		if (state.cannotContinue) {
			std::mutex localMutex;
			std::unique_lock<std::mutex> lock(localMutex);

			// Ждем либо continue, либо stop
			state.continueCV.wait(lock, [&]() {
				return !state.cannotContinue || state.terminated;
				});

			if (state.terminated) {
				// Заполняем нулями помеченные элементы (по условию)
				std::lock_guard<std::mutex> lock(arrayMutex);
				for (int j = 0; j < arraySize; j++) {
					if (array[j] == threadNumber) {
						array[j] = 0;
					}
				}
				return;
			}
		}
	}
}

void inputNatural(int& integer, int max = INT_MAX) {
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

int main() {
	int count;

	// 1. Захват памяти под массив
	std::cout << "Enter the array size:" << std::endl;
	inputNatural(arraySize);

	// 2. Инициализация массива нулями
	array = std::make_unique<int[]>(arraySize);
	for (int i = 0; i < arraySize; i++) {
		array[i] = 0;
	}

	// 3. Запрос количества потоков marker
	std::cout << "Enter the number of marker threads:" << std::endl;
	inputNatural(count);

	// Инициализация состояний потоков
	threadStates.reserve(count);
	for (int i = 0; i < count; i++) {
		threadStates.push_back(std::make_unique<ThreadState>());
	}

	markerThreads.reserve(count);

	// 4. Запуск потоков marker
	for (int i = 0; i < count; i++) {
		markerThreads.emplace_back(markerThread, i + 1);
	}

	// 5. Сигнал на начало работы
	threadStartFlag = true;

	int activeThreads = count;

	// 6. Главный цикл
	while (activeThreads > 0) {
		// 6.1. Ждем, пока все потоки не подадут сигналы о невозможности продолжения
		bool allWaiting = false;
		while (!allWaiting && activeThreads > 0) {
			allWaiting = true;
			for (int i = 0; i < count; i++) {
				if (!threadStates[i]->terminated && !threadStates[i]->cannotContinue) {
					allWaiting = false;
					break;
				}
			}
			if (!allWaiting) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}

		if (activeThreads == 0) break;

		// 6.2. Вывод массива
		printArray(array.get(), arraySize);

		// 6.3. Запрос номера потока для завершения
		int threadToTerminate;
		std::cout << "Enter thread number to terminate (1-" << count << "): ";
		inputNatural(threadToTerminate, count);

		int index = threadToTerminate - 1;

		// 6.4. Сигнал на завершение выбранному потоку
		threadStates[index]->terminated = true;
		threadStates[index]->cannotContinue = false;
		threadStates[index]->continueCV.notify_one();

		// 6.5. Ожидание завершения потока
		if (markerThreads[index].joinable()) {
			markerThreads[index].join();
		}

		// 6.6. Вывод массива после завершения
		printArray(array.get(), arraySize);

		// 6.7. Сигнал на продолжение оставшимся потокам
		for (int i = 0; i < count; i++) {
			if (!threadStates[i]->terminated) {
				threadStates[i]->cannotContinue = false;
				threadStates[i]->continueCV.notify_one();
			}
		}

		activeThreads--;
	}

	// 7. Завершение работы
	std::cout << "All threads completed. Program finished." << std::endl;
	return 0;
}