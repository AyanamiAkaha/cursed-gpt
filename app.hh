#pragma once

#include <vector>

#include "ncwindow.hh"

class Chat;

struct CommandResult {
    bool _exit;
    int exit_code;
};

constexpr CommandResult NO_COMMAND = { false, 0 };

class App
{
private:
    std::vector<Chat> chats;
    int current_chat = 0;
    std::string input_buffer;
    bool needs_refresh = true;
    NCWindow window;

    CommandResult processInput();
    CommandResult parseCommand(std::string command);
    void newChat();
public:
    App();
    ~App();
    int run();
};