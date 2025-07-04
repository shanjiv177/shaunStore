#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP

#include <atomic>

class RedisServer {
    private:
        int port;
        int serverSocket;
        std::atomic<bool> running;

    public:
        RedisServer(int port);
        ~RedisServer() = default;  
        
        void shutdown();
        void run();
};

#endif