#pragma once
#include <windows.h>
#include <string>
#include <iostream>
#include <climits>
#include <cfloat>

const std::string PIPE_NAME = "\\\\.\\pipe\\EmployeePipe";
const int NAME_SIZE = 30;
const int MESSAGE_SIZE = 255;

struct Employee {
	int num;
	char name[NAME_SIZE + 1];
	double hours;
};

enum class RequestType {
	READ,
	MODIFY
};

struct Request {
	RequestType type;
	int employeeNum;
};

struct Response {
	bool found;
	Employee record;
	char message[MESSAGE_SIZE + 1];
};

enum class EndAction {
	SAVE,
	FINISH
};

inline void inputNatural(int& integer, int max = INT_MAX) {
	while (true) {
		std::cin >> integer;

		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			std::cout << "Invalid input. Enter an integer 0 < " << max << ": ";
			continue;
		}
		if (integer <= 0 || integer > max) {
			std::cout << "Invalid input. Enter an integer 0 < " << max << ": ";
			continue;
		}

		std::cin.ignore(INT_MAX, '\n');
		break;
	}
}

inline void inputDouble(double& real, double min = 0.0, double max = DBL_MAX) {
	while (true) {
		std::cin >> real;
		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			std::cout << "Invalid input. Enter a number (" << min << " - " << max << "): ";
			continue;
		}
		if (real < min || real > max) {
			std::cout << "Invalid input. Enter a number (" << min << " - " << max << "): ";
			continue;
		}

		std::cin.ignore(INT_MAX, '\n');
		break;
	}
}