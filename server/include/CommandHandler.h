#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>

class CommandHandler {

    private:


    public:
        CommandHandler();

        std::string handleCommand(const std::vector<std::string>& parsedCommand);
        bool parseRESP(const std::string& buffer, std::vector<std::string>& tokens, size_t& parsedLen);
        bool parseArray(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);
        bool parseBulkString(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);
        bool parseSimpleString(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);
        bool parseInteger(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);
        bool parseError(const std::string& buffer, std::vector<std::string>& tokens, size_t& pos);

};

#endif