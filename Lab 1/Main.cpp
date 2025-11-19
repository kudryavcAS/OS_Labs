#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <vector>
#include "employee.h"

using std::cin;
using std::cout;

void printBinFile(const std::string& fileName) {

	std::ifstream fin(fileName, std::ios::binary);
	if (!fin) {
		cout << "Cannot open binary file " << fileName << "\n";
		return;
	}

	employee person;
	cout << "\n\tBinary file content:\n";
	cout << std::left << std::setw(15) << "Employee ID";
	cout << std::left << std::setw(15) << "Employee name";
	cout << std::left << std::setw(15) << "Employee hours\n";
	while (fin.read(reinterpret_cast<char*>(&person), sizeof(person))) {
		cout << std::left << std::setw(15) << person.num
			<< std::setw(15) << person.name
			<< std::setw(15) << person.hours
			<< "\n";
	}
	fin.close();
}

void printReportFile(const std::string& fileName) {

	std::ifstream fin(fileName);
	if (!fin) {
		cout << "Cannot open report file " << fileName << "\n";
		return;
	}

	std::string line;
	while (std::getline(fin, line)) {
		cout << line << "\n";
	}
	fin.close();
}

void runProcess(const std::string& cmd) {

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	std::wstring wcmd(cmd.begin(), cmd.end());

	if (!CreateProcess(
		NULL, &wcmd[0], NULL, NULL,
		FALSE, 0, NULL, NULL, &si, &pi
	)) {
		throw std::runtime_error("Не удалось запустить процесс: " + cmd);
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

int main() {

	std::iostream::sync_with_stdio(false);
	std::cin.tie(0);
	std::cout.tie(0);

	std::string binFileName;
	int count;

	cout << "Enter the name of the binary file:\n";
	cin >> binFileName;
	cout << "Enter the number of entries:\n";
	cin >> count;

	try {
		std::string creatorToCmd = "Creator.exe " + binFileName + " " + std::to_string(count);
		runProcess(creatorToCmd);
		printBinFile(binFileName);

		std::string reportFileName;
		double payPerHour;

		cout << "Enter the name of the peport file:\n";
		cin >> reportFileName;
		cout << "Enter the payment per hour:\n";
		cin >> payPerHour;

		std::string reportedToCmd = "Reporter.exe " + binFileName + " " + reportFileName + " " + std::to_string(payPerHour);
		runProcess(reportedToCmd);
		printReportFile(reportFileName);
	}
	catch (const std::exception& exp) {

		cout << "Error: " << exp.what() << "\n";
	}

}