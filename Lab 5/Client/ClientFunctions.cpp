#include "Client.hpp"

void printEmployee(const Employee& e) {
	std::cout << "ID: " << e.num
		<< ", Name: " << std::string(e.name)
		<< ", Hours: " << e.hours << "\n";
}

HANDLE connectToServer() {
	if (!WaitNamedPipe(PIPE_NAME.c_str(), NMPWAIT_WAIT_FOREVER)) {
		return INVALID_HANDLE_VALUE;
	}

	HANDLE hPipe = CreateFile(
		PIPE_NAME.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (hPipe != INVALID_HANDLE_VALUE) {
		DWORD mode = PIPE_READMODE_MESSAGE;
		SetNamedPipeHandleState(hPipe, &mode, NULL, NULL);
	}

	return hPipe;
}

void processSession(HANDLE hPipe) {
	bool running = true;
	while (running) {
		std::cout << "Select option:\n1. Modify record\n2. Read record\n3. Exit\n> ";
		int choice;
		inputNatural(choice, 3);

		if (choice == 3) {
			running = false;
			break;
		}
		if (choice != 1 && choice != 2) continue;

		Request req;
		req.type = (choice == 1) ? RequestType::MODIFY : RequestType::READ;

		std::cout << "Enter Employee ID: ";
		inputNatural(req.employeeNum);

		DWORD bytesWritten, bytesRead;
		if (!WriteFile(hPipe, &req, sizeof(Request), &bytesWritten, NULL)) {
			std::cerr << "Connection lost (Write request).\n";
			break;
		}

		Response resp;
		std::cout << "Waiting for server (lock might be active)...\n";

		if (!ReadFile(hPipe, &resp, sizeof(Response), &bytesRead, NULL)) {
			std::cerr << "Connection lost (Read response).\n";
			break;
		}

		if (!resp.found) {
			std::cout << "Server message: " << std::string(resp.message) << "\n";
			continue;
		}

		printEmployee(resp.record);

		EndAction action = EndAction::FINISH;
		Employee newEmp = resp.record;

		if (req.type == RequestType::MODIFY) {
			std::cout << "\n[MODIFY MODE]\n";
			std::cout << "Current Name: " << std::string(newEmp.name) << ". Enter new Name: ";
			std::string tempName;
			std::getline(std::cin, tempName);

			strncpy_s(newEmp.name, tempName.c_str(), NAME_SIZE);

			std::cout << "Current Hours: " << newEmp.hours << ". Enter new Hours: ";
			inputDouble(newEmp.hours);

			std::cout << "Press 'y' to save changes, any other key to cancel: ";
			char c; std::cin >> c;
			if (c == 'y' || c == 'Y') {
				action = EndAction::SAVE;
			}
		}
		else {
			std::cout << "\n[READ MODE] Press any key to release lock and finish.";
			(void)_getch();
		}

		if (!WriteFile(hPipe, &action, sizeof(EndAction), &bytesWritten, NULL)) break;

		if (action == EndAction::SAVE) {
			WriteFile(hPipe, &newEmp, sizeof(Employee), &bytesWritten, NULL);
			std::cout << "Modification sent.\n";
		}
		else {
			std::cout << "Session finished.\n";
		}
	}
}