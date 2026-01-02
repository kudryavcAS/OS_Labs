#define NOMINMAX
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <vector>
#include <limits>
#include "employee.h"

const int COL_WIDTH = 15;

template <typename T>
T getValidNumber(const std::string& prompt, T minVal = 0, T maxVal = std::numeric_limits<T>::max()) {
	T value;
	while (true) {
		std::cout << prompt;

		if (std::cin >> value) {
			if (value >= minVal && value <= maxVal) {
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				return value;
			}
			else {
				std::cout << "Input out of range. Try again.\n";
			}
		}
		else {
			std::cout << "Invalid input. Please enter a number.\n";
			std::cin.clear();
		}
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
}

void printBinFile(const std::string& fileName) {
	std::ifstream fin(fileName, std::ios::binary);
	if (!fin) {
		std::cout << "Cannot open binary file " << fileName << "\n";
		return;
	}

	employee person;
	std::cout << "\n\tBinary file content:\n";
	std::cout << std::left << std::setw(COL_WIDTH) << "Employee ID";
	std::cout << std::left << std::setw(COL_WIDTH) << "Employee name";
	std::cout << std::left << std::setw(COL_WIDTH) << "Employee hours\n";

	while (fin.read(reinterpret_cast<char*>(&person), sizeof(person))) {
		std::cout << std::left << std::setw(COL_WIDTH) << person.num
			<< std::setw(COL_WIDTH) << person.name
			<< std::setw(COL_WIDTH) << person.hours
			<< "\n";
	}
	fin.close();
}

void printReportFile(const std::string& fileName) {
	std::ifstream fin(fileName);
	if (!fin) {
		std::cout << "Cannot open report file " << fileName << "\n";
		return;
	}

	std::cout << "\n\tReport file content:\n";
	std::string line;
	while (std::getline(fin, line)) {
		std::cout << line << "\n";
	}
	fin.close();
}

void runProcess(const std::string& cmd) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	std::wstring wcmd(cmd.begin(), cmd.end());

	if (!CreateProcess(
		NULL,
		&wcmd[0],
		NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE,
		NULL, NULL, &si, &pi
	)) {
		throw std::runtime_error("Failed to start process. Error code: " + std::to_string(GetLastError()));
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

int main() {
	std::iostream::sync_with_stdio(false);

	try {
		std::string binFileName;
		std::cout << "Enter the name of the binary file: ";
		std::cin >> binFileName;

		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		int count = getValidNumber<int>("Enter the number of entries: ");

		std::string creatorCmd = "Creator.exe " + binFileName + " " + std::to_string(count);

		std::cout << "Starting Creator...\n";
		runProcess(creatorCmd);

		printBinFile(binFileName);

		std::string reportFileName;
		std::cout << "\nEnter the name of the report file: ";
		std::cin >> reportFileName;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		double payPerHour = getValidNumber<double>("Enter the payment per hour: ");

		std::string reporterCmd = "Reporter.exe " + binFileName + " " + reportFileName + " " + std::to_string(payPerHour);

		std::cout << "Starting Reporter...\n";
		runProcess(reporterCmd);

		printReportFile(reportFileName);
	}
	catch (const std::exception& exp) {
		std::cout << "Error: " << exp.what() << "\n";
	}

	std::cout << "\n";
	system("pause");
	return 0;
}