#include "../include/RedisServer.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

static RedisServer* Server = nullptr;

RedisServer::RedisServer(int port) : port(port), serverSocket(-1), running(true) {
    Server = this;
}

void RedisServer::shutdown() {
    running = false;
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
    std::cout << "Server shutdown complete." << std::endl;
}

void RedisServer::run() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    int val = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
        std::cerr << "Failed to set socket options." << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind socket." << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return;
    }

    if (listen(serverSocket, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on socket." << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return;
    }

    std::cout << "Server is running on port " << port << std::endl;
}