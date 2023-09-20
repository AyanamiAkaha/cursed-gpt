#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <ncurses.h>
#include "app.hh"
#include "chat.hh"
#include "ncwindow.hh"

#define PROJECT_NAME "gpt-chat"

App::App(/* args */) {
    newChat();
    window.setChat(chats.at(current_chat));
    window.setStatusCb([this]() {
        return "Chat "
            + std::to_string(this->current_chat + 1)
            + "/" + std::to_string(this->chats.size());
    });
}

App::~App() {
    window.setStatusCb([]() { return ""; });
}

CommandResult App::parseCommand(std::string command) {
    if (!command.size()) return { true, 0 };
    if(command == "/q") return { true, 0 };
    if(command.at(0) == '/') {
        chats.at(current_chat)->addString("Unknown command: " + command + "\n");
    } else {
        chats.at(current_chat)->send(command);
    }
    return { false, 0 };
}

void App::newChat() {
    auto chat = std::make_shared<Chat>(PROJECT_NAME);
    chats.push_back(chat);
    window.setChat(chat);
}

int App::run() {
    while(true) {
        auto maybeCmdStr = window.processInput();
        if(maybeCmdStr) {
            auto result = parseCommand(maybeCmdStr.value());
            if(result._exit) return result.exit_code;
        }
        window.update();
    }
}