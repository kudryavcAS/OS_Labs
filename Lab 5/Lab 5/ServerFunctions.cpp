#include "Server.hpp"

std::mutex g_consoleMtx;
std::map<int, std::unique_ptr<std::shared_timed_mutex>> g_recordLocks;
std::mutex g_locksMapMtx;

std::shared_timed_mutex* getMutexForID(int id) {
	std::lock_guard<std::mutex> mapLock(g_locksMapMtx);
	if (g_recordLocks.find(id) == g_recordLocks.end()) {
		g_recordLocks[id] = std::make_unique<std::shared_timed_mutex>();
	}
	return g_recordLocks[id].get();
}

void log(const std::string& msg) {
	std::lock_guard<std::mutex> lock(g_consoleMtx);
	std::cout << "[SERVER Thread " << std::this_thread::get_id() << "]: " << msg << "\n";
}

void printFileContent(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) return;

	Employee e;

	std::lock_guard<std::mutex> lock(g_consoleMtx);
	std::cout << "\n--- File Content ---\n";
	std::cout << std::left << std::setw(10) << "ID" << std::setw(35) << "Name" << std::setw(10) << "Hours" << "\n";
	std::cout << std::string(55, '-') << "\n";

	while (file.read(reinterpret_cast<char*>(&e), sizeof(Employee))) {
		std::cout << std::left << std::setw(10) << e.num
			<< std::setw(35) << e.name << std::setw(10) << e.hours << "\n";
	}
	std::cout << std::string(55, '-') << "\n";
	file.close();
}

long long findRecordOffset(std::fstream& file, int id) {
	file.clear();
	file.seekg(0, std::ios::beg);

	Employee emp;
	long long currentPos = 0;

	while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
		if (emp.num == id) {
			return currentPos;
		}
		currentPos = file.tellg();
	}
	return -1;
}

void clientHandler(HANDLE hPipe, std::string dbFileName) {
	std::fstream file(dbFileName, std::ios::binary | std::ios::in | std::ios::out);

	if (!file.is_open()) {
		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);
		return;
	}

	DWORD bytesRead, bytesWritten;
	Request req;

	while (ReadFile(hPipe, &req, sizeof(Request), &bytesRead, NULL)) {
		if (bytesRead == 0) break;

		Response resp;
		long long offset = findRecordOffset(file, req.employeeNum);

		if (offset == -1) {
			resp.found = false;
			strncpy_s(resp.message, "Employee not found.", MESSAGE_SIZE);
			WriteFile(hPipe, &resp, sizeof(Response), &bytesWritten, NULL);
			continue;
		}

		auto* recordMutex = getMutexForID(req.employeeNum);

		if (req.type == RequestType::MODIFY) {
			recordMutex->lock();
		}
		else {
			recordMutex->lock_shared();
		}

		file.clear();
		file.seekg(offset);

		if (!file.read(reinterpret_cast<char*>(&resp.record), sizeof(Employee))) {
			if (req.type == RequestType::MODIFY) recordMutex->unlock();
			else recordMutex->unlock_shared();

			resp.found = false;
			strncpy_s(resp.message, "Read error.", MESSAGE_SIZE);
			WriteFile(hPipe, &resp, sizeof(Response), &bytesWritten, NULL);
			continue;
		}

		resp.found = true;
		strncpy_s(resp.message, "OK", MESSAGE_SIZE);
		WriteFile(hPipe, &resp, sizeof(Response), &bytesWritten, NULL);

		EndAction action;
		if (ReadFile(hPipe, &action, sizeof(EndAction), &bytesRead, NULL)) {
			if (req.type == RequestType::MODIFY && action == EndAction::SAVE) {
				Employee modifiedEmp;
				if (ReadFile(hPipe, &modifiedEmp, sizeof(Employee), &bytesRead, NULL)) {
					file.clear();
					file.seekp(offset);
					file.write(reinterpret_cast<char*>(&modifiedEmp), sizeof(Employee));
					file.flush();
				}
			}
		}

		if (req.type == RequestType::MODIFY) {
			recordMutex->unlock();
		}
		else {
			recordMutex->unlock_shared();
		}
	}

	file.close();
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
}

void createDatabase(std::string& outFilename, int& outClientCount) {
	std::cout << "Enter filename for database: ";
	std::cin >> outFilename;

	std::ofstream file(outFilename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to create file!\n";
		exit(1);
	}

	int empCount;
	std::cout << "Enter number of employees: ";
	inputNatural(empCount);

	for (int i = 0; i < empCount; ++i) {
		Employee emp;
		std::cout << "Employee " << (i + 1) << "\nID: ";
		inputNatural(emp.num);
		std::cout << "Name (max " << NAME_SIZE << " chars): ";
		std::string tempName;
		std::getline(std::cin, tempName);
		strncpy_s(emp.name, tempName.c_str(), NAME_SIZE);
		std::cout << "Hours: ";
		inputDouble(emp.hours);

		file.write(reinterpret_cast<char*>(&emp), sizeof(Employee));
	}
	file.close();

	printFileContent(outFilename);

	std::cout << "\nEnter number of clients to launch: ";
	inputNatural(outClientCount);
}

void launchClients(int count) {
	for (int i = 0; i < count; ++i) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		std::string cmdLine = "Client.exe " + std::to_string(i + 1);
		std::vector<char> cmdBuf(cmdLine.begin(), cmdLine.end());
		cmdBuf.push_back(0);

		if (CreateProcess(NULL, cmdBuf.data(), NULL, NULL, FALSE,
			CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
}

void runServer(const std::string& dbFileName, int clientCount) {
	std::vector<std::thread> threads;

	for (int i = 0; i < clientCount; ++i) {
		HANDLE hPipe = CreateNamedPipe(
			PIPE_NAME.c_str(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			1024, 1024, 0, NULL
		);

		if (hPipe == INVALID_HANDLE_VALUE) continue;

		if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
			log("Client " + std::to_string(i + 1) + " connected.");
			threads.emplace_back(clientHandler, hPipe, dbFileName);
		}
		else {
			CloseHandle(hPipe);
		}
	}

	for (auto& t : threads) {
		if (t.joinable()) t.join();
	}
}