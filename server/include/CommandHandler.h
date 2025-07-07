#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "../include/Database.h"

class CommandHandler {

    private:


    public:
        CommandHandler();

        std::string handleCommand(const std::vector<std::string>& parsedCommand);

        std::string handlePing(const std::vector<std::string>& args, Database& db);
        std::string handleEcho(const std::vector<std::string>& args, Database& db);
        std::string handleFlushAll(const std::vector<std::string>& args, Database& db);
        std::string handleSet(const std::vector<std::string>& args, Database& db);
        std::string handleGet(const std::vector<std::string>& args, Database& db);
        std::string handleKeys(const std::vector<std::string>& args, Database& db);
        std::string handleType(const std::vector<std::string>& args, Database& db);
        std::string handleDel(const std::vector<std::string>& args, Database& db);
        std::string handleExists(const std::vector<std::string>& args, Database& db);
        std::string handleRename(const std::vector<std::string>& args, Database& db);
        std::string handleExpiry(const std::vector<std::string>& args, Database& db);

        bool parseRESP(const std::string& buffer, std::vector<std::string>& tokens, size_t& parsedLen);
        bool parseArray(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);
        bool parseBulkString(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);
        bool parseSimpleString(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);
        bool parseInteger(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);
        bool parseError(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);

};

#endif