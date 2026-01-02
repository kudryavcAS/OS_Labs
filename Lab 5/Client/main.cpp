#include "Client.hpp"

int main(int argc, char* argv[]) {
	std::string title = "Client";

	if (argc > 1) {
		title += " " + std::string(argv[1]);
	}

#ifdef _WIN32
	SetConsoleTitle(title.c_str());
#endif

	HANDLE hPipe = connectToServer();

	if (hPipe == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to connect to server. Error: " << GetLastError() << "\n";
		(void)_getch();
		return 1;
	}

	processSession(hPipe);

	CloseHandle(hPipe);
	return 0;
}