#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <ncurses.h>
#include <map>
#include <fstream>
#include <json.hpp>

#include "app.hh"
#include "chat.hh"
#include "ncwindow.hh"
#include "gpt_chat.hh"

#define PROJECT_NAME "gpt-chat"

std::map<std::string, std::function<void(App*, std::string)>> Command::commands;

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

void App::newChat() {
    auto chat = std::make_shared<GptChat>(PROJECT_NAME + std::to_string(chats.size()));
    chats.push_back(chat);
    current_chat = chats.size() - 1;
    window.setChat(chat);
}

void App::nextChat() {
    current_chat = (current_chat + 1) % chats.size();
    window.setChat(chats.at(current_chat));
}

void App::prevChat() {
    current_chat = (current_chat - 1) % chats.size();
    window.setChat(chats.at(current_chat));
}

void App::chatNum(int num) {
    current_chat = (num - 1) % chats.size();
    window.setChat(chats.at(current_chat));
}

void App::saveCurrentChat(std::string filename) {
    auto msgs = chats.at(current_chat)->getMessages();
    auto json = nlohmann::json::object();
    json["template"] = chats.at(current_chat)->getName();
    auto jsonMsgs = nlohmann::json::array();
    for (auto msg: msgs) {
        jsonMsgs.push_back({
            {"id", msg.id},
            {"timestamp", msg.timestamp},
            {"message", msg.message},
            {"author", (int)msg.author}
        });
    }
    json["messages"] = jsonMsgs;
    auto file = std::ofstream(filename);
    file << json.dump(2);
    file.close();
}

int App::run() {
    while(!_exit.first) {
        auto maybeCmdStr = window.processInput();
        if(maybeCmdStr) {
            if (maybeCmdStr.value()[0] == '/') {
                auto cmdStr = maybeCmdStr.value().substr(1);
                auto cmd = cmdStr.substr(0, cmdStr.find(' '));
                auto args = cmdStr.substr(cmdStr.find(' ') + 1);
                if (!Command::run(cmd, this, args)) {
                    chats.at(current_chat)->log("Unknown command: " + cmd);
                }
            } else {
                chats.at(current_chat)->send(maybeCmdStr.value(), Author::USER);
            }
        }
        for(auto chat: chats) {
            chat->checkReceived();
        }
        window.update();
    }
    return _exit.second;
}

void App::quit(int exit_code) {
    _exit = { true, exit_code };
}