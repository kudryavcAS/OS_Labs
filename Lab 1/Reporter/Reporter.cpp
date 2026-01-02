#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include "employee.h"

int main(int argc, char* argv[]) {
	std::iostream::sync_with_stdio(false);

	const int COL_WIDTH = 15;

	if (argc != 4) {
		std::cout << "Error: Invalid number of arguments.\n";
		std::cout << "Usage: Reporter.exe <BinaryFile> <ReportFile> <PaymentPerHour>\n";
		system("pause");
		return 1;
	}

	try {
		std::string binFileName = argv[1];
		std::string reportFileName = argv[2];
		double xPerHour = std::stod(argv[3]);

		std::ifstream in(binFileName, std::ios::binary);
		if (!in) {
			std::cout << "Error: cannot open binary file " << binFileName << "\n";
			system("pause");
			return 1;
		}

		std::ofstream out(reportFileName);
		if (!out) {
			std::cout << "Error: cannot open report file " << reportFileName << "\n";
			in.close();
			system("pause");
			return 1;
		}

		out << "\tReport on the file \"" << binFileName << "\":\n";
		out << std::left << std::setw(COL_WIDTH) << "Employee ID";
		out << std::left << std::setw(COL_WIDTH) << "Employee name";
		out << std::left << std::setw(COL_WIDTH) << "Employee hours";
		out << std::left << std::setw(COL_WIDTH) << "Employee salary\n";

		employee person;
		while (in.read(reinterpret_cast<char*>(&person), sizeof(employee))) {
			double salary = person.hours * xPerHour;
			out << std::left << std::setw(COL_WIDTH) << person.num
				<< std::setw(COL_WIDTH) << person.name
				<< std::setw(COL_WIDTH) << person.hours
				<< std::setw(COL_WIDTH) << std::fixed << std::setprecision(2) << salary
				<< "\n";
		}

		in.close();
		out.close();

		std::cout << "Report generated successfully in " << reportFileName << "\n";
	}
	catch (const std::exception& e) {
		std::cout << "Runtime error: " << e.what() << "\n";
	}

	system("pause");
	return 0;
}