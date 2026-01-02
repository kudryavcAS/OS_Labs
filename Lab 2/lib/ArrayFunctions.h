#pragma once
#include <vector>
#include <windows.h>

struct ArrayData {
    const std::vector<int>* array;
    int maxElement, minElement, average;

    ArrayData(const std::vector<int>* _array, int _max, int _min, int _average);
    ArrayData();
};

DWORD WINAPI searchMinMaxElement(LPVOID lpData);
DWORD WINAPI searchAverage(LPVOID lpData);