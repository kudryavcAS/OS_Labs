#include "std_thread.h"

int arraySize = 0;
std::unique_ptr<int[]> array;

std::atomic<bool> threadStartFlag{ false };

struct ThreadState {
	std::condition_variable continueCV;
	std::atomic<bool> cannotContinue{ false };
	std::atomic<bool> terminated{ false };
};

std::vector<std::unique_ptr<ThreadState>> threadStates;
std::vector<std::thread> markerThreads;

void markerThread(int threadNumber) {
	int markedCount = 0;
	int index = threadNumber - 1;
	auto& state = *threadStates[index];

	srand(threadNumber);

	while (!threadStartFlag) {
		std::this_thread::yield();
	}

	while (true) {
		int randomValue = rand();
		int i = randomValue % arraySize;

		{
			std::lock_guard<std::mutex> lock(arrayMutex);
			if (array[i] == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				
				array[i] = threadNumber;
				markedCount++;

				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
			else {
				std::lock_guard<std::mutex> consoleLock(consoleMutex);
				std::cout << "Number of thread: " << threadNumber
					<< ". Count of marked elements: " << markedCount
					<< ". Index of unmarked elements: " << i << std::endl;

				state.cannotContinue = true;
			}
		}

		if (state.cannotContinue) {
			std::mutex localMutex;
			std::unique_lock<std::mutex> lock(localMutex);

			state.continueCV.wait(lock, [&]() {
				return !state.cannotContinue || state.terminated;
				});

			if (state.terminated) {
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

int main() {
	int count;

	std::cout << "Enter the array size:" << std::endl;
	inputNatural(arraySize);

	array = std::unique_ptr<int[]>(new int[arraySize]);
	for (int i = 0; i < arraySize; i++) {
		array[i] = 0;
	}

	std::cout << "Enter the number of marker threads:" << std::endl;
	inputNatural(count, 64);

	markerThreads.reserve(count);
	threadStates.reserve(count);

	for (int i = 0; i < count; i++) {
		threadStates.push_back(std::unique_ptr<ThreadState>(new ThreadState()));
	}
	for (int i = 0; i < count; i++) {
		markerThreads.emplace_back(markerThread, i + 1);
	}

	threadStartFlag = true;
	int activeThreads = count;

	while (activeThreads > 0) {
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

		printArray(array.get(), arraySize);

		int threadToTerminate;
		std::cout << "Enter thread number to terminate (1-" << count << "): ";
		inputNatural(threadToTerminate, count);

		int index = threadToTerminate - 1;

		threadStates[index]->terminated = true;
		threadStates[index]->cannotContinue = false;
		threadStates[index]->continueCV.notify_one();

		if (markerThreads[index].joinable()) {
			markerThreads[index].join();
		}

		printArray(array.get(), arraySize);

		for (int i = 0; i < count; i++) {
			if (!threadStates[i]->terminated) {
				threadStates[i]->cannotContinue = false;
				threadStates[i]->continueCV.notify_one();
			}
		}

		activeThreads--;
	}

	std::cout << "All threads completed. Program finished." << std::endl;
	return 0;
}