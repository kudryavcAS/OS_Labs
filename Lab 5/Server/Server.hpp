#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <memory>
#include <iomanip>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <map>

#include "Entities.hpp" 

void log(const std::string& msg);
void printFileContent(const std::string& filename);
long long findRecordOffset(std::fstream& file, int id);
void createDatabase(std::string& outFilename, int& outClientCount);
void launchClients(int count);
void clientHandler(HANDLE hPipe, std::string dbFileName);
void runServer(const std::string& dbFileName, int clientCount);