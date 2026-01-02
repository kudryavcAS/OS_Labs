#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <limits>
#include "employee.h"

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
				std::cout << "Input out of range (" << minVal << " - " << maxVal << "). Try again.\n";
			}
		}
		else {
			std::cout << "Invalid input. Please enter a number.\n";
			std::cin.clear();
		}

		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
}

int main(int argc, char* argv[]) {
	std::iostream::sync_with_stdio(false);

	if (argc != 3) {
		std::cout << "Error: Invalid number of arguments.\n";
		std::cout << "Usage: Creator.exe <Filename> <Count>\n";
		system("pause");
		return 1;
	}

	try {
		std::string filename = argv[1];
		int count = std::stoi(argv[2]);

		std::ofstream out(filename, std::ios::binary);
		if (!out) {
			std::cout << "Error: cannot open file " << filename << " for writing\n";
			system("pause");
			return 1;
		}

		for (int i = 0; i < count; i++) {
			employee person;

			std::cout << "--- Person #" << i + 1 << " ---\n";

			person.num = getValidNumber<int>("Enter ID: ");

			std::cout << "Enter name (max 9 chars): ";
			std::cin >> std::setw(10) >> person.name;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			person.hours = getValidNumber<double>("Enter working hours: ");

			out.write(reinterpret_cast<char*>(&person), sizeof(employee));
			std::cout << "\n";
		}

		out.close();
		std::cout << "File " << filename << " created successfully.\n";
	}
	catch (const std::exception& e) {
		std::cout << "Runtime error: " << e.what() << "\n";
	}

	system("pause");
	return 0;
}