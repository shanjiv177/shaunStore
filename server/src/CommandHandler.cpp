#include "../include/CommandHandler.h"
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


// Handles the  command and returns the response.
std::string CommandHandler::handleCommand(const std::vector<std::string>& parsedCommand) {
    if (parsedCommand.empty()) {
        return "-Error: Empty Command\r\n"; // Return error in RESP format
    }

    std::string cmd = parsedCommand[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    std::ostringstream response;

    // Connect to DB

    // Handle the command
    if (cmd == "ping") {
        response << "+PONG\r\n"; // RESP format for PING command
    } else if (cmd == "echo") {
        if (parsedCommand.size() != 2) {
            response << "-ERR: Wrong number of arguments for 'echo' command\r\n"; // Return error in RESP
        }
        else {
            response << "$" << parsedCommand[1].size() << "\r\n" << parsedCommand[1] << "\r\n"; // RESP format for ECHO command
        }
    }
    else {
        response << "-ERR: Unknown command '" << cmd << "'\r\n"; // Return error in RESP format
    }

    return response.str();
}


