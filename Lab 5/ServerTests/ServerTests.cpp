#include "pch.h"
#include "CppUnitTest.h"
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdio>
#include <sstream>

#include "Server.hpp"
#include "Entities.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ServerTests
{
	TEST_CLASS(ServerCoreTests)
	{
	private:
		const std::string TEST_FILENAME = "test_db.bin";

		void CreateTestFileWithEmployees(const std::vector<Employee>& employees) {
			std::ofstream file(TEST_FILENAME, std::ios::binary);
			for (const auto& emp : employees) {
				file.write(reinterpret_cast<const char*>(&emp), sizeof(Employee));
			}
			file.close();
		}

	public:
		TEST_METHOD_CLEANUP(Teardown) {
			remove(TEST_FILENAME.c_str());
		}

		TEST_METHOD(Test01_EmployeeStructSize) {
			size_t size = sizeof(Employee);
			size_t minExpected = sizeof(int) + 31 + sizeof(double);
			Assert::IsTrue(size >= minExpected);
		}

		TEST_METHOD(Test02_FindOffset_EmptyFile) {
			std::ofstream ofs(TEST_FILENAME);
			ofs.close();

			std::fstream file(TEST_FILENAME, std::ios::binary | std::ios::in | std::ios::out);
			long long offset = findRecordOffset(file, 1);
			file.close();

			Assert::AreEqual(-1LL, offset);
		}

		TEST_METHOD(Test03_FindOffset_MultipleRecords) {
			std::vector<Employee> data(3);
			data[0] = { 1, "User1", 10 };
			data[1] = { 2, "User2", 20 };
			data[2] = { 3, "User3", 30 };
			CreateTestFileWithEmployees(data);

			std::fstream file(TEST_FILENAME, std::ios::binary | std::ios::in | std::ios::out);
			long long offset = findRecordOffset(file, 2);
			file.close();

			long long expectedOffset = sizeof(Employee);
			Assert::AreEqual(expectedOffset, offset);
		}

		TEST_METHOD(Test07_Protocol_Read) {
			Employee e{ 10, "Reader", 50.0 };
			CreateTestFileWithEmployees({ e });

			std::string pipeName = "\\\\.\\pipe\\UnitTestPipe_Read";
			HANDLE hServerPipe = CreateNamedPipe(pipeName.c_str(), PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 1024, 1024, 0, NULL);

			HANDLE hClientPipe = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			DWORD mode = PIPE_READMODE_MESSAGE;
			SetNamedPipeHandleState(hClientPipe, &mode, NULL, NULL);

			std::thread serverWorker(clientHandler, hServerPipe, TEST_FILENAME);

			Request req;
			req.type = RequestType::READ;
			req.employeeNum = 10;
			DWORD written, read;
			WriteFile(hClientPipe, &req, sizeof(Request), &written, NULL);

			Response resp;
			ReadFile(hClientPipe, &resp, sizeof(Response), &read, NULL);

			Assert::IsTrue(resp.found);
			Assert::AreEqual(10, resp.record.num);

			EndAction action = EndAction::FINISH;
			WriteFile(hClientPipe, &action, sizeof(EndAction), &written, NULL);

			CloseHandle(hClientPipe);
			if (serverWorker.joinable()) serverWorker.join();
		}
	};

	TEST_CLASS(InputValidationTests)
	{
	public:
		TEST_METHOD(Test_InputNatural_Valid)
		{
			std::stringstream ss("10\n");
			std::streambuf* oldCin = std::cin.rdbuf(ss.rdbuf());

			int value = 0;
			inputNatural(value);

			std::cin.rdbuf(oldCin);

			Assert::AreEqual(10, value);
		}

		TEST_METHOD(Test_InputNatural_InvalidThenValid)
		{
			std::stringstream ss("abc\n-5\n25\n");

			std::stringstream outSS;
			std::streambuf* oldCin = std::cin.rdbuf(ss.rdbuf());
			std::streambuf* oldCout = std::cout.rdbuf(outSS.rdbuf());

			int value = 0;
			inputNatural(value);

			std::cin.rdbuf(oldCin);
			std::cout.rdbuf(oldCout);

			Assert::AreEqual(25, value);
		}

		TEST_METHOD(Test_InputNatural_MaxLimit)
		{
			std::stringstream ss("5\n2\n");

			std::stringstream outSS;
			std::streambuf* oldCin = std::cin.rdbuf(ss.rdbuf());
			std::streambuf* oldCout = std::cout.rdbuf(outSS.rdbuf());

			int value = 0;
			inputNatural(value, 3);

			std::cin.rdbuf(oldCin);
			std::cout.rdbuf(oldCout);

			Assert::AreEqual(2, value);
		}

		TEST_METHOD(Test_InputDouble_Valid)
		{
			std::stringstream ss("40.5\n");
			std::streambuf* oldCin = std::cin.rdbuf(ss.rdbuf());

			double value = 0.0;
			inputDouble(value);

			std::cin.rdbuf(oldCin);

			Assert::AreEqual(40.5, value);
		}

		TEST_METHOD(Test_InputDouble_Invalid)
		{
			std::stringstream ss("xyz\n-10.0\n12.5\n");

			std::stringstream outSS;
			std::streambuf* oldCin = std::cin.rdbuf(ss.rdbuf());
			std::streambuf* oldCout = std::cout.rdbuf(outSS.rdbuf());

			double value = 0.0;
			inputDouble(value);

			std::cin.rdbuf(oldCin);
			std::cout.rdbuf(oldCout);

			Assert::AreEqual(12.5, value);
		}
	};
}