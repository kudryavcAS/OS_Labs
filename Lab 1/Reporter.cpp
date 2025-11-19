#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "employee.h"

int main(int argc, char* argv[]) {
   std::iostream::sync_with_stdio(false);
   std::cin.tie(0);
   std::cout.tie(0);

   using std::string;
   using std::cin;
   using std::cout;

   string binFileName = argv[1];
   string reportFileName = argv[2];
   double xPerHour = std::stod(argv[3]);

   std::ifstream in(binFileName, std::ios::binary);
   if (!in) {
      cout << "Error: cannot open file " << binFileName << " for reading\n";
      return 1;
   }

   // Открываем текстовый файл для отчёта
   std::ofstream out(reportFileName);
   if (!out) {
      cout << "Error: cannot open file " << reportFileName << " for writing\n";
      return 1;
   }

   out << "\tReport on the file \"" << binFileName << "\":\n";
   out << std::left << std::setw(15) << "Employee ID";
   out << std::left << std::setw(15) << "Employee name";
   out << std::left << std::setw(15) << "Employee hours";
   out << std::left << std::setw(15) << "Employee salary\n";
   
   employee person;
   while (in.read(reinterpret_cast<char*>(&person), sizeof(employee))) {
      double salary = person.hours * xPerHour;
      out << std::left << std::setw(15) << person.num
          << std::setw(15) << person.name
          <<std:: setw(15) << person.hours
          << std::setw(15) << std::fixed << std::setprecision(2) << salary
          << "\n";
   }

   in.close();
   out.close();
   return 0;
}