// OS_Lab3.h : включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта.

#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <memory>
#include <condition_variable>


extern std::mutex arrayMutex;
extern std::mutex consoleMutex;

void inputNatural(int& integer, int max = INT_MAX);
void printArray(int* array, int arraySize);
// TODO: установите здесь ссылки на дополнительные заголовки, требующиеся для программы.
