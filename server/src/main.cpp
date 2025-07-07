#include "../include/Server.h"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {

    int PORT = argc > 1 ? std::stoi(argv[1]) : 6379;

    Server server(PORT);

    // // Background processes to dump db
    // std::thread Persistence([] () {
    //     while (true) {
    //         std::this_thread::sleep_for(std::chrono::seconds(300));
    //         // Dump the db
    //     }
    // });
    // Persistence.detach();

    server.run();

    return 0;
}