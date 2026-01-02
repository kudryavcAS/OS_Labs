#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "employee.h"
#include <fstream>
#include <string>
#include <cstdio>

void cleanUp(const std::string& filename) {
    std::remove(filename.c_str());
}

TEST_CASE("Employee structure size and integrity") {
    CHECK(sizeof(employee) >= 22);

    employee e;
    e.num = 1;
    e.hours = 10.5;
    strncpy_s(e.name, "TestUser", 9);

    CHECK(e.num == 1);
    CHECK(e.hours == 10.5);
    CHECK(std::string(e.name) == "TestUser");
}

TEST_CASE("Binary file Read/Write logic") {
    std::string filename = "test_data.bin";

    employee original;
    original.num = 101;
    strncpy_s(original.name, "Worker", 9);
    original.hours = 40.0;

    std::ofstream out(filename, std::ios::binary);
    REQUIRE(out.is_open());
    out.write(reinterpret_cast<char*>(&original), sizeof(employee));
    out.close();

    employee loaded;
    std::ifstream in(filename, std::ios::binary);
    REQUIRE(in.is_open());
    in.read(reinterpret_cast<char*>(&loaded), sizeof(employee));
    in.close();

    CHECK(loaded.num == original.num);
    CHECK(std::string(loaded.name) == std::string(original.name));
    CHECK(loaded.hours == original.hours);

    cleanUp(filename);
}

TEST_CASE("Salary calculation logic") {
    double hours = 50.0;
    double rate = 15.5;
    double expectedSalary = 775.0;

    double actualSalary = hours * rate;

    CHECK(actualSalary == doctest::Approx(expectedSalary));
}