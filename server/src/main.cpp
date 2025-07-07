#include "../include/Server.h"
#include "../include/Database.h"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {

    int PORT = argc > 1 ? std::stoi(argv[1]) : 6379;

    Server server(PORT);

    if (!Database::getInstance().loadDatabase("dump")) {
        std::cerr << "Failed to load database." << std::endl;
    } else {
        std::cout << "Database loaded successfully from dump" << std::endl;
    }

    // Background processes to dump db
    std::thread Persistence([] () {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            // Dump the db
            if (Database::getInstance().dumpDatabase("dump")) {
                std::cout << "Database dumped successfully." << std::endl;
            } else {
                std::cerr << "Failed to dump database." << std::endl;
            }
        }
    });
    Persistence.detach();

    server.run();

    return 0;
}