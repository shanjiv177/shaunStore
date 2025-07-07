#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <chrono>

class Database {
    private:
        Database() = default; // Private constructor to prevent instantiation
        ~Database() = default; // Private destructor to prevent deletion
        Database(const Database&) = delete; // Prevent copy construction
        Database& operator=(const Database&) = delete; // Prevent assignment

        std::mutex db_mutex; // Mutex for thread safety

        std::unordered_map<std::string, std::string> keyValueStore; // Key-Value pairs
        std::unordered_map<std::string, std::vector<std::string>> listStore;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hashStore;

        std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiryStore; // Store for key expirations

    public:
        static Database& getInstance();

        
        bool dumpDatabase(const std::string& filename);
        bool loadDatabase(const std::string& filename);

        bool flushAll();

        bool set(const std::string& key, const std::string& value);
        std::string get(const std::string& key);
        std::vector<std::string> keys();
        std::string type(const std::string& key);
        bool del(const std::string& key);
        bool exists(const std::string& key);
        bool rename(const std::string& oldKey, const std::string& newKey);

        bool expiry(const std::string& key, int seconds);

        void purgeExpired();

        // List Operations
        ssize_t llen(const std::string& key);
        std::string lindex(const std::string& key, int index);
        void lpush(const std::string& key, const std::string& value);
        void rpush(const std::string& key, const std::string& value);
        std::string lpop(const std::string& key);
        std::string rpop(const std::string& key);
        int lrem(const std::string& key, int count, const std::string& value);
        bool lset(const std::string& key, int index, const std::string& value);
        std::vector<std::string> lget(const std::string& key);
};

#endif