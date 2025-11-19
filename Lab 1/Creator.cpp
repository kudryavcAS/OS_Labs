#include <fstream>
#include <string>
#include <iostream>
#include "employee.h"

int main(int argc, char* argv[]) {

	std::iostream::sync_with_stdio(false);
	std::cin.tie(0);
	std::cout.tie(0);

	std::string filename = argv[1];
	int count = std::stoi(argv[2]);

	std::ofstream out(filename, std::ios::binary);

	for (int i = 0; i < count; i++) {
		employee person;

		std::cout << "Person #: " << i + 1 << "\nEnter the employee's identification number:\n";
		std::cin >> person.num;

		std::cout << "Enter the employee's name (maximum 9 letters):\n";
		std::cin >> person.name;

		std::cout << "Enter the number of working hours:\n";
		std::cin >> person.hours;

		out.write(reinterpret_cast<char*>(&person), sizeof(employee));
	}

	out.close();
	return 0;
}