#include "NetworkGameServer.h"
#include <iostream>
#include <signal.h>
#include <chrono>
#include <thread>

volatile bool running = true;

void signalHandler([[maybe_unused]] int signum) {
    running = false;
}

int main() {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	auto& server = NetworkGameServer::getInstance();
    
	if (!server.initialize(7777)) {
		std::cerr << "Failed to initialize server" << std::endl;
		return 1;
	}

	std::cout << "Server started on port 7777" << std::endl;

	// main loop of server
	while (running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	server.stop();
	std::cout << "Server stopped" << std::endl;
    
	return 0;
}