#include "../include/CommandHandler.h"
#include "../include/Database.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

CommandHandler::CommandHandler() {}

bool CommandHandler::parseSimpleString(const std::string& buffer, std::vector<std::string>& parsedCommand, size_t& pos) {
    if (pos >= buffer.size() || buffer[pos] != '+') {
        return false; // Invalid format
    }
    pos++; // Skip the '+'
    size_t endPos = buffer.find("\r\n", pos);
    if (endPos == std::string::npos) {
        return false; // Invalid format
    }
    parsedCommand.push_back(buffer.substr(pos, endPos - pos));
    return true;
}

bool CommandHandler::parseBulkString(const std::string& buffer, std::vector<std::string>& parsedCommand, size_t& pos) {
    if (pos >= buffer.size() || buffer[pos] != '$') {
        return false; // Invalid format
    }
    pos++; // Skip the '$'
    size_t crlf = buffer.find("\r\n", pos);
    if (crlf == std::string::npos) {
        return false; // Invalid format
    }
    int length = std::stoi(buffer.substr(pos, crlf - pos));
    pos = crlf + 2; // Move past \r\n
    if (length < 0) {
        parsedCommand.push_back(""); // Null bulk string
    } else {
        if (pos + length > buffer.size()) {
            return false; // Invalid format
        }
        parsedCommand.push_back(buffer.substr(pos, length));
        pos += length + 2; // Move past the string and \r\n
    }
    return true;
}

bool CommandHandler::parseInteger(const std::string& buffer, std::vector<std::string>& parsedCommand, size_t& pos) {
    if (pos >= buffer.size() || buffer[pos] != ':') {
        return false; // Invalid format
    }
    pos++; // Skip the ':'
    size_t endPos = buffer.find("\r\n", pos);
    if (endPos == std::string::npos) {
        return false; // Invalid format
    }
    parsedCommand.push_back(buffer.substr(pos, endPos - pos));
    pos = endPos + 2; // Move past \r\n
    return true;
}

bool CommandHandler::parseError(const std::string& buffer, std::vector<std::string>& parsedCommand, size_t& pos) {
    if (pos >= buffer.size() || buffer[pos] != '-') {
        return false; // Invalid format
    }
    pos++; // Skip the '-'
    size_t endPos = buffer.find("\r\n", pos);
    if (endPos == std::string::npos) {
        return false; // Invalid format
    }
    parsedCommand.push_back(buffer.substr(pos, endPos - pos));
    pos = endPos + 2; // Move past \r\n
    return true;
}

bool CommandHandler::parseArray(const std::string& buffer, std::vector<std::string>& parsedCommand, size_t& pos) {
    if (pos >= buffer.size() || buffer[pos] != '*') {
        return false; // Invalid format
    }
    pos++; // Skip the '*'
    size_t crlf = buffer.find("\r\n", pos);
    if (crlf == std::string::npos) {
        return false; // Invalid format
    }
    int numElements = std::stoi(buffer.substr(pos, crlf - pos));
    pos = crlf + 2; // Move past \r\n

    for (int i = 0; i < numElements; i++) {
        if (pos >= buffer.size()) {
            return false; // Invalid format
        }
        if (buffer[pos] == '$') {
            if (!parseBulkString(buffer, parsedCommand, pos)) {
                return false; // Invalid format
            }
        } else if (buffer[pos] == '+') {
            if (!parseSimpleString(buffer, parsedCommand, pos)) {
                return false; // Invalid format
            }
        } else if (buffer[pos] == ':') {
            if (!parseInteger(buffer, parsedCommand, pos)) {
                return false; // Invalid format
            }
        } else if (buffer[pos] == '-') {
            if (!parseError(buffer, parsedCommand, pos)) {
                return false; // Invalid format
            }
        } else if (buffer[pos] == '*') {
            if (!parseArray(buffer, parsedCommand, pos)) {
                return false;
            }
        }
        else {
            return false; // Invalid RESP format
        }
    }
    return true;
}

bool CommandHandler::parseRESP(const std::string& buffer, std::vector<std::string>& parsedCommand, size_t& parsedLen) {
    parsedCommand.clear();
    parsedLen = 0;

    if (buffer.empty()) {
        return false; // Empty buffer
    }
    std::cout << "Parsing RESP: " << buffer << std::endl;
    size_t pos = 0;
    if (buffer[pos] == '*') {
        if (!parseArray(buffer, parsedCommand, pos)) {
            return false; // Invalid RESP format
        }
    } else if (buffer[pos] == '$') {
        if (!parseBulkString(buffer, parsedCommand, pos)) {
            return false; // Invalid RESP format
        }
    } else if (buffer[pos] == '+') {
        if (!parseSimpleString(buffer, parsedCommand, pos)) {
            return false; // Invalid RESP format
        }
    } else if (buffer[pos] == ':') {
        if (!parseInteger(buffer, parsedCommand, pos)) {
            return false; // Invalid RESP format
        }
    } else if (buffer[pos] == '-') {
        if (!parseError(buffer, parsedCommand, pos)) {
            return false; // Invalid RESP format
        }
    } else {
        return false; // Invalid RESP format
    }

    parsedLen = pos; // Set parsed length to current position
    std::cout << "Parsed RESP length: " << parsedLen << std::endl;
    std::cout << "Parsed RESP tokens: ";
    for (const auto& token : parsedCommand) {
        std::cout << token << " ";
    }
    return true;
}

std::string CommandHandler::handlePing(const std::vector<std::string>& args, Database& db) {
    return "+PONG\r\n"; // RESP format for PING command
}

std::string CommandHandler::handleEcho(const std::vector<std::string>& args, Database& db) {
    if (args.size() != 2) {
        return "-ERR: Wrong number of arguments for 'echo' command\r\n"; // Return error in RESP format
    }
    return "$" + std::to_string(args[1].size()) + "\r\n" + args[1] + "\r\n"; // RESP format for ECHO command
}

std::string CommandHandler::handleFlushAll(const std::vector<std::string>& args, Database& db) {
    db.flushAll();
    return "+OK\r\n"; // RESP format for successful FLUSHALL command
}

std::string CommandHandler::handleSet(const std::vector<std::string>& args, Database& db) {
    if (args.size() != 3) {
        return "-ERR: Wrong number of arguments for 'set' command\r\n"; // Return error in RESP format
    }
    db.set(args[1], args[2]);
    return "+OK\r\n"; // RESP format for successful SET command
}

std::string CommandHandler::handleGet(const std::vector<std::string>& args, Database& db) {
    if (args.size() != 2) {
        return "-ERR: Wrong number of arguments for 'get' command\r\n"; // Return error in RESP format
    }
    std::string value = db.get(args[1]);
    if (value.empty()) {
        return "$-1\r\n"; // Null bulk string for non-existing key
    }
    return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n"; // RESP format for GET command
}

std::string CommandHandler::handleKeys(const std::vector<std::string>& args, Database& db) {
    std::vector<std::string> keys = db.keys();
    std::ostringstream response;
    response << "*" << keys.size() << "\r\n";
    for (const auto& key : keys) {
        response << "$" << key.size() << "\r\n" << key << "\r\n"; // RESP format for KEYS command
    }
    return response.str();
}

std::string CommandHandler::handleType(const std::vector<std::string>& args, Database& db) {
    if (args.size() != 2) {
        return "-ERR: Wrong number of arguments for 'type' command\r\n"; // Return error in RESP format
    }
    std::string type = db.type(args[1]);
    return "+TYPE " + type + "\r\n"; // RESP format for TYPE command
}

std::string CommandHandler::handleDel(const std::vector<std::string>& args, Database& db) {
    if (args.size() != 2) {
        return "-ERR: Wrong number of arguments for 'del' command\r\n"; // Return error in RESP format
    }
    bool deleted = db.del(args[1]);
    return (deleted ? ":1\r\n" : ":0\r\n"); // RESP format for DEL command
}

std::string CommandHandler::handleExists(const std::vector<std::string>& args, Database& db) {
    if (args.size() != 2) {
        return "-ERR: Wrong number of arguments for 'exists' command\r\n"; // Return error in RESP format
    }
    bool exists = db.exists(args[1]);
    return (exists ? ":1\r\n" : ":0\r\n"); // RESP format for EXISTS command
}

std::string CommandHandler::handleRename(const std::vector<std::string>& args, Database& db) {
    if (args.size() != 3) {
        return "-ERR: Wrong number of arguments for 'rename' command\r\n"; // Return error in RESP format
    }
    bool renamed = db.rename(args[1], args[2]);
    return (renamed ? "+OK\r\n" : "-ERR: Key does not exist\r\n"); // RESP format for RENAME command
}

std::string CommandHandler::handleExpiry(const std::vector<std::string>& args, Database& db) {
    if (args.size() != 3) {
        return "-ERR: Wrong number of arguments for 'expire' command\r\n"; // Return error in RESP format
    }
    int seconds = std::stoi(args[2]);
    bool expirySet = db.expiry(args[1], seconds);
    return (expirySet ? "+OK\r\n" : "-ERR: Key does not exist\r\n"); // RESP format for EXPIRE command
}


// Handles the  command and returns the response.
std::string CommandHandler::handleCommand(const std::vector<std::string>& parsedCommand) {
    if (parsedCommand.empty()) {
        return "-Error: Empty Command\r\n"; // Return error in RESP format
    }

    std::string cmd = parsedCommand[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    std::ostringstream response;

    // Connect to DB
    Database& db = Database::getInstance();

    // Handle the command
    if (cmd == "ping") {
        response << handlePing(parsedCommand, db);
    } else if (cmd == "echo") {
        response << handleEcho(parsedCommand, db);
    } else if (cmd == "flushall") {
        response << handleFlushAll(parsedCommand, db);
    } else if (cmd == "set") {
        response << handleSet(parsedCommand, db);
    } else if (cmd == "get") {
        response << handleGet(parsedCommand, db);
    } else if (cmd == "keys") {
        response << handleKeys(parsedCommand, db);
    } else if (cmd == "type") {
        response << handleType(parsedCommand, db);
    } else if (cmd == "del") {
        response << handleDel(parsedCommand, db);
    } else if (cmd == "exists") {
        response << handleExists(parsedCommand, db);
    } else if (cmd == "rename") {
        response << handleRename(parsedCommand, db);
    } else if (cmd == "expire" || cmd == "ttl") {
        response << handleExpiry(parsedCommand, db);
    }
    else {
        response << "-ERR: Unknown command '" << cmd << "'\r\n"; // Return error in RESP format
    }

    return response.str();
}


