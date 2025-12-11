#include "pch.h"
#include "CppUnitTest.h"
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <chrono>
#include <thread>

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
			HANDLE hFile = CreateFile(TEST_FILENAME.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			Assert::AreNotEqual((void*)INVALID_HANDLE_VALUE, (void*)hFile);

			for (const auto& emp : employees) {
				DWORD written;
				WriteFile(hFile, &emp, sizeof(Employee), &written, NULL);
			}

			CloseHandle(hFile);
		}

	public:

		TEST_METHOD_INITIALIZE(Setup)
		{
			initializeSync();
		}

		TEST_METHOD_CLEANUP(Teardown)
		{
			remove(TEST_FILENAME.c_str());
			cleanupSync();
		}

		TEST_METHOD(Test01_EmployeeStructSize)
		{
			size_t size = sizeof(Employee);
			size_t minExpected = sizeof(int) + 31 + sizeof(double);

			Assert::IsTrue(size >= minExpected);
		}

		TEST_METHOD(Test02_FindOffset_EmptyFile)
		{
			HANDLE hFile = CreateFile(TEST_FILENAME.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			Assert::AreNotEqual((void*)INVALID_HANDLE_VALUE, (void*)hFile);

			long long offset = findRecordOffset(hFile, 1);

			CloseHandle(hFile);

			Assert::AreEqual(-1LL, offset);
		}

		TEST_METHOD(Test03_FindOffset_SingleRecord)
		{
			Employee e;
			e.num = 10;
			strcpy_s(e.name, "TestUser");
			e.hours = 100.0;

			CreateTestFileWithEmployees({ e });

			HANDLE hFile = CreateFile(TEST_FILENAME.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			long long offset = findRecordOffset(hFile, 10);
			CloseHandle(hFile);

			Assert::AreEqual(0LL, offset);
		}

		TEST_METHOD(Test04_FindOffset_MultipleRecords)
		{
			std::vector<Employee> data(3);
			data[0] = { 1, "User1", 10 };
			data[1] = { 2, "User2", 20 };
			data[2] = { 3, "User3", 30 };

			CreateTestFileWithEmployees(data);

			HANDLE hFile = CreateFile(TEST_FILENAME.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			long long offset = findRecordOffset(hFile, 2);
			CloseHandle(hFile);

			long long expectedOffset = sizeof(Employee);

			Assert::AreEqual(expectedOffset, offset);
		}

		TEST_METHOD(Test05_FindOffset_NotFound)
		{
			std::vector<Employee> data(2);
			data[0] = { 1, "User1", 10 };
			data[1] = { 2, "User2", 20 };

			CreateTestFileWithEmployees(data);

			HANDLE hFile = CreateFile(TEST_FILENAME.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			long long offset = findRecordOffset(hFile, 999);
			CloseHandle(hFile);

			Assert::AreEqual(-1LL, offset);
		}

		TEST_METHOD(Test06_FileStructureIntegration)
		{
			std::vector<Employee> data = {
				{ 101, "Alice", 40.5 },
				{ 102, "Bob", 32.0 }
			};
			CreateTestFileWithEmployees(data);

			std::ifstream file(TEST_FILENAME, std::ios::binary);
			Assert::IsTrue(file.is_open());

			Employee temp;

			file.read(reinterpret_cast<char*>(&temp), sizeof(Employee));
			Assert::AreEqual(101, temp.num);
			Assert::AreEqual(std::string("Alice"), std::string(temp.name));
			Assert::AreEqual(40.5, temp.hours);

			file.read(reinterpret_cast<char*>(&temp), sizeof(Employee));
			Assert::AreEqual(102, temp.num);

			file.close();
		}
		TEST_METHOD(Test07_Protocol_Read)
		{
			Employee e{ 10, "Reader", 50.0 };
			CreateTestFileWithEmployees({ e });

			std::string pipeName = "\\\\.\\pipe\\UnitTestPipe_Read";
			HANDLE hServerPipe = CreateNamedPipe(pipeName.c_str(), PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 1024, 1024, 0, NULL);

			Assert::AreNotEqual((void*)INVALID_HANDLE_VALUE, (void*)hServerPipe);

			HANDLE hClientPipe = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			Assert::AreNotEqual((void*)INVALID_HANDLE_VALUE, (void*)hClientPipe);

			DWORD mode = PIPE_READMODE_MESSAGE;
			SetNamedPipeHandleState(hClientPipe, &mode, NULL, NULL);

			auto params = new ThreadParam;
			params->hPipe = hServerPipe;
			params->dbFileName = TEST_FILENAME;

			std::thread serverWorker(clientThread, params);

			Request req;
			req.type = RequestType::READ;
			req.employeeNum = 10;
			DWORD written, read;

			Assert::IsTrue(WriteFile(hClientPipe, &req, sizeof(Request), &written, NULL));

			Response resp;
			Assert::IsTrue(ReadFile(hClientPipe, &resp, sizeof(Response), &read, NULL), L"Failed to read response");

			Assert::IsTrue(resp.found);
			Assert::AreEqual(10, resp.record.num);
			Assert::AreEqual(50.0, resp.record.hours);

			EndAction action = EndAction::FINISH;
			Assert::IsTrue(WriteFile(hClientPipe, &action, sizeof(EndAction), &written, NULL));

			CloseHandle(hClientPipe);

			if (serverWorker.joinable()) serverWorker.join();
		}

		TEST_METHOD(Test08_Protocol_Modify)
		{
			Employee e{ 20, "OldName", 10.0 };
			CreateTestFileWithEmployees({ e });

			std::string pipeName = "\\\\.\\pipe\\UnitTestPipe_Modify";
			HANDLE hServerPipe = CreateNamedPipe(pipeName.c_str(), PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 1024, 1024, 0, NULL);

			HANDLE hClientPipe = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

			DWORD mode = PIPE_READMODE_MESSAGE;
			SetNamedPipeHandleState(hClientPipe, &mode, NULL, NULL);

			auto params = new ThreadParam;
			params->hPipe = hServerPipe;
			params->dbFileName = TEST_FILENAME;

			std::thread serverWorker(clientThread, params);

			Request req{ RequestType::MODIFY, 20 };
			DWORD written, read;
			Assert::IsTrue(WriteFile(hClientPipe, &req, sizeof(Request), &written, NULL));

			Response resp;
			Assert::IsTrue(ReadFile(hClientPipe, &resp, sizeof(Response), &read, NULL), L"Failed to read response");
			Assert::IsTrue(resp.found);

			EndAction action = EndAction::SAVE;
			Assert::IsTrue(WriteFile(hClientPipe, &action, sizeof(EndAction), &written, NULL));

			Employee newEmp = resp.record;
			strcpy_s(newEmp.name, "NewName");
			newEmp.hours = 99.9;
			Assert::IsTrue(WriteFile(hClientPipe, &newEmp, sizeof(Employee), &written, NULL));

			CloseHandle(hClientPipe);
			if (serverWorker.joinable()) serverWorker.join();

			std::ifstream file(TEST_FILENAME, std::ios::binary);
			Employee fromDisk;
			file.read(reinterpret_cast<char*>(&fromDisk), sizeof(Employee));

			Assert::AreEqual(std::string("NewName"), std::string(fromDisk.name));
			Assert::AreEqual(99.9, fromDisk.hours);
			file.close();
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