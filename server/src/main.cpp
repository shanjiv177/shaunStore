#include "../include/RedisServer.h"
#include <iostream>

int main(int argc, char* argv[]) {

    int PORT = argc > 1 ? std::stoi(argv[1]) : 6379;

    RedisServer server(PORT);
    server.run();

    return 0;
}