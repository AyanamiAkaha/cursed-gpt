#pragma once

#include <vector>

class Chat;

// WINDOW type needed by ncurses
struct _win_st;
typedef struct _win_st WINDOW;


struct CommandResult {
    bool _exit;
    int exit_code;
};

constexpr CommandResult NO_COMMAND = { false, 0 };

class App
{
private:
    std::vector<Chat> chats;
    uint32_t current_chat = 0;
    WINDOW* w_prompt;
    WINDOW* w_chat;
    WINDOW* w_title;
    WINDOW* w_status;
    std::string input_buffer;
    bool needs_refresh = true;

    void initCurses();
    CommandResult processInput();
    CommandResult parseCommand(std::string command);
    void printChat();
    void newChat();
public:
    App();
    ~App();
    int run();
};