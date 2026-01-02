#include "Server.hpp"

int main() {
#ifdef _WIN32
	SetConsoleTitle("Server");
#endif

	std::string filename;
	int clientCount = 0;

	createDatabase(filename, clientCount);
	launchClients(clientCount);

	runServer(filename, clientCount);

	std::cout << "\nAll clients finished. Final result:";
	printFileContent(filename);

	std::cout << "Press any key to exit...";
	char c;
	std::cin >> c;

	return 0;
}