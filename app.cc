#include <iostream>
#include "app.hh"

#define PROJECT_NAME "gpt-chat"

CommandResult App::parseCommand(std::string command) {
    if (!command.size()) return { true, 0 };
    if(command == "\\q") return { true, 0 };
    if(command.at(0) == '\\') {
        std::cout << "Unknown command: " << command << "\n";
    } else {
        std::cout << command << "\n";
    }
    return { false, 0 };
}

std::string App::prompt() const {
    return "> ";
}

App::App(/* args */)
{
}

App::~App()
{
}

int App::run() {
    std::cout << "Welcome to " << PROJECT_NAME << "!\n";
    while(true) {
        std::string cmd;
        std::cout << prompt();
        std::cin >> cmd;
        auto result = parseCommand(cmd);
        if (result._exit) {
            std::cout << "\n"; // ensure that after exiting bash prompt will start in new line
            return result.exit_code;
        }
    }
}