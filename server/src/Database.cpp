#include "../include/Database.h"
#include <mutex>
#include <fstream>
#include <sstream>
#include <string> 
#include <vector>
#include <mutex>
#include <unordered_map>

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

/*
We will handle three types of data:
1. Key-Value pairs
2. Lists
3. Hashes

Format for dumping and loading the database:
K key value
L key item1 item2 item3 ...
H key field1 value1 field2 value2 ...
*/

bool Database::dumpDatabase(const std::string& filename) {
    // Implement the logic to dump the database to a file
    std::cout << "Dumping database to " << filename << std::endl;
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return false;
    }

    for (const auto& kv : keyValueStore) {
        ofs << "K " <<  kv.first << " " << kv.second << "\n";
    }

    for (const auto& list : listStore) {
        ofs << "L " << list.first << " ";
        for (const auto& item : list.second) {
            ofs << item << " ";
        }
        ofs << "\n";
    }

    for (const auto& hash : hashStore) {
        ofs << "H " << hash.first << " ";
        for (const auto& field : hash.second) {
            ofs << field.first << " " << field.second << " ";
        } 
        ofs << "\n";
    }

    return true;
}

bool Database::loadDatabase(const std::string& filename) {
    // Implement the logic to load the database from a file
    std::cout << "Loading database from " << filename << std::endl;
    
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return false;
    }

    keyValueStore.clear();
    listStore.clear();
    hashStore.clear();

    std::string line;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "K") {
            std::string key, value;
            iss >> key >> value;
            keyValueStore[key] = value;
        } else if (type == "L") {
            std::string key;
            iss >> key;
            std::vector<std::string> listItems;
            std::string item;
            while (iss >> item) {
                listItems.push_back(item);
            }
            listStore[key] = listItems;
        } else if (type == "H") {
            std::string key, field, value;
            iss >> key;
            while (iss >> field >> value) {
                hashStore[key][field] = value;
            }
        }
    }

    return true;
}

// FLUSHALL
bool Database::flushAll() {
    std::lock_guard<std::mutex> lock(db_mutex);
    keyValueStore.clear();
    listStore.clear();
    hashStore.clear();
    return true;
}

// Key Value Store Operations
bool Database::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    keyValueStore[key] = value;
    return true;
}

std::string Database::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    auto it = keyValueStore.find(key);
    if (it != keyValueStore.end()) {
        return it->second;
    }
    return ""; // Return empty string if key does not exist
}

std::vector<std::string> Database::keys() {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    std::vector<std::string> keysList;
    for (const auto& kv : keyValueStore) {
        keysList.push_back(kv.first);
    }
    return keysList;
}

std::string Database::type(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    if (keyValueStore.find(key) != keyValueStore.end()) {
        return "string";
    } else if (listStore.find(key) != listStore.end()) {
        return "list";
    } else if (hashStore.find(key) != hashStore.end()) {
        return "hash";
    }
    return "none"; // Return "none" if key does not exist
}

bool Database::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    if (keyValueStore.erase(key) > 0) {
        if (expiryStore.find(key) != expiryStore.end()) {
            expiryStore.erase(key); // Remove from expiry store if it exists
        }
        return true; // Key was found and deleted
    } else if (listStore.erase(key) > 0) {
        if (expiryStore.find(key) != expiryStore.end()) {
            expiryStore.erase(key); // Remove from expiry store if it exists
        }
        return true; // Key was found and deleted
    } else if (hashStore.erase(key) > 0) {
        if (expiryStore.find(key) != expiryStore.end()) {
            expiryStore.erase(key); // Remove from expiry store if it exists
        }
        return true; // Key was found and deleted
    }
    return false; // Key does not exist
}

bool Database::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    if (keyValueStore.find(key) != keyValueStore.end() || listStore.find(key) != listStore.end() || hashStore.find(key) != hashStore.end()) {
        return true;
    }
    return false;
}

bool Database::rename(const std::string& oldKey, const std::string& newKey) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    bool found = false;
    if (keyValueStore.find(oldKey) != keyValueStore.end()) {
        keyValueStore[newKey] = keyValueStore[oldKey];
        keyValueStore.erase(oldKey);
        found = true;
    } else if (listStore.find(oldKey) != listStore.end()) {
        listStore[newKey] = listStore[oldKey];
        listStore.erase(oldKey);
        found = true;
    } else if (hashStore.find(oldKey) != hashStore.end()) {
        hashStore[newKey] = hashStore[oldKey];
        hashStore.erase(oldKey);
        found = true;
    }

    if (expiryStore.find(oldKey) != expiryStore.end()) {
        expiryStore[newKey] = expiryStore[oldKey];
        expiryStore.erase(oldKey);
    }
    return found;
}

ssize_t Database::llen(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (listStore.find(key) != listStore.end()) {
        return listStore[key].size();
    }
    else {
        return -1;
    }
}

std::string Database::lindex(const std::string& key, int index) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (listStore.find(key) != listStore.end()) {
        const auto& list = listStore[key];
        if (index < 0) {
            index += list.size(); // Handle negative index
        }
        if (index >= 0 && index < static_cast<int>(list.size())) {
            return list[index];
        }
    }
    return ""; // Return empty string if key does not exist or index is out of range
}

std::string Database::lpop(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (listStore.find(key) != listStore.end() && !listStore[key].empty()) {
        std::string value = listStore[key].front(); // Get the first element
        listStore[key].erase(listStore[key].begin()); // Remove the first element
        return value;
    }
    return ""; // Return empty string if key does not exist or list is empty
}

std::string Database::rpop(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (listStore.find(key) != listStore.end() && !listStore[key].empty()) {
        std::string value = listStore[key].back(); // Get the last element
        listStore[key].pop_back(); // Remove the last element
        return value;
    }
    return ""; // Return empty string if key does not exist or list is empty
}

bool Database::lset(const std::string& key, int index, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (listStore.find(key) != listStore.end()) {
        std::vector<std::string>& list = listStore[key]; // Use reference to modify in place
        if (index < 0) {
            index = static_cast<int>(list.size()) + index; // Support negative indexing
        }
        if (index < 0 || index >= static_cast<int>(list.size())) {
            return false;
        }
        list[index] = value;
        return true;
    }
    return false; // Key not found
}

void Database::lpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    listStore[key].insert(listStore[key].begin(), value);
}

void Database::rpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    listStore[key].push_back(value);
}

int Database::lrem(const std::string& key, int count, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    int removedCount = 0;
    if (listStore.find(key) != listStore.end()) {
        std::vector<std::string> list = listStore[key];
        if (count == 0) {
            for (auto it = list.begin(); it != list.end();) {
                if (*it == value) {
                    it = list.erase(it); // Remove the item and get the next iterator
                    removedCount++;
                } else {
                    ++it;
                }
            }
        }
        else if (count < 0) {
            for (auto it = list.rbegin(); it != list.rend() && removedCount < -count;) {
                if (*it == value) {
                    auto normalIt = it.base();
                    --normalIt;
                    normalIt = list.erase(normalIt); // Remove the item and get the next iterator
                    it = std::vector<std::string>::reverse_iterator(normalIt); // Update reverse iterator
                    removedCount++;
                } else {
                    ++it;
                }
            }
        } else {
            for (auto it = list.begin(); it != list.end() && removedCount < count;) {
                if (*it == value) {
                    it = list.erase(it); // Remove the item and get the next iterator
                    removedCount++;
                } else {
                    ++it;
                }
            }
        }

        if (list.empty()) {
            listStore.erase(key); // Remove the key if the list is empty
        } else {
            listStore[key] = list; // Update the list in the store
        }
    }

    return removedCount; // Return the number of removed items
}

std::vector<std::string> Database::lget(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (listStore.find(key) != listStore.end()) {
        return listStore[key]; // Return the list if it exists
    }
    return {};
}

bool Database::expiry(const std::string& key, int seconds) {
    std::lock_guard<std::mutex> lock(db_mutex);
    purgeExpired();
    bool exists = keyValueStore.find(key) != keyValueStore.end() ||
                     listStore.find(key) != listStore.end() ||
                     hashStore.find(key) != hashStore.end();
    if (!exists) {
        return false; // Key does not exist
    }
    if (seconds <= 0) {
        // If seconds is 0 or negative, remove the key from expiryStore
        expiryStore.erase(key);
    }
    else {
        // Set the expiry time to now + seconds
        expiryStore[key] = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    }
    return true;
}


void Database::purgeExpired() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = expiryStore.begin(); it != expiryStore.end();) {
        if (it->second <= now) {
            // If the key has expired, remove it from all stores
            keyValueStore.erase(it->first);
            listStore.erase(it->first);
            hashStore.erase(it->first);
            it = expiryStore.erase(it); // Remove from expiry store and get next iterator
        } else {
            ++it; // Move to the next item
        }
    }
}




