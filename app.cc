#include <iostream>
#include <memory>
#include <vector>
#include <ncurses.h>
#include "app.hh"
#include "chat.hh"
#include "ncwindow.hh"

#define PROJECT_NAME "gpt-chat"

App::App(/* args */) {
    newChat();
}

App::~App() {
    signal(SIGWINCH, SIG_DFL);
}

CommandResult App::processInput() {
    auto ch = getch();
    if (ch == ERR) return NO_COMMAND;
    if (ch == KEY_RESIZE) {
        window.resize();
        fullRefresh();
        return NO_COMMAND;
    }
    if (ch == '\n') {
        auto result = parseCommand(input_buffer);
        input_buffer.clear();
        window.clearPrompt();
        return result;
    } else {
        input_buffer.push_back(ch);
        window.feedback(ch);
        return NO_COMMAND;
    }
}

CommandResult App::parseCommand(std::string command) {
    if (!command.size()) return { true, 0 };
    if(command == "\\q") return { true, 0 };
    if(command.at(0) == '\\') {
        needs_refresh = true;
        chats.at(current_chat).addString("Unknown command: " + command + "\n");
    } else {
        needs_refresh = true;
        chats.at(current_chat).send(command);
    }
    return { false, 0 };
}

void App::newChat() {
    chats.emplace_back();
}

void App::fullRefresh() {
    setStatus();
    setTitle();
    window.redraw();
    needs_refresh = true;
}

void App::setStatus() {
    window.setStatus("Chat " + std::to_string(current_chat + 1) + "/" + std::to_string(chats.size()));
}

void App::setTitle() {
    window.setTitle(PROJECT_NAME);
}

int App::run() {
    setStatus();
    setTitle();
    window.redraw();
    while(true) {
        auto result = processInput();
        if(result._exit) return result.exit_code;
        if(needs_refresh) {
            window.printChat(chats.at(current_chat));
            needs_refresh = false;
        }
    }
}