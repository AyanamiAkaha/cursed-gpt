#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <ncurses.h>
#include <map>
#include <fstream>
#include <json.hpp>
#include <filesystem>

#include "app.hh"
#include "chat.hh"
#include "ncwindow.hh"
#include "gpt_chat.hh"

#define PROJECT_NAME "cursed-gpt"
#define CONFIG_DIR "." PROJECT_NAME

std::map<std::string, std::function<void(App*, std::string)>> Command::commands;

App::App(/* args */) {
    newChat();
    window.setChat(chats.at(current_chat));
    window.setStatusCb([this]() {
        return "Chat "
            + std::to_string(this->current_chat + 1)
            + "/" + std::to_string(this->chats.size())
            + " [" + this->chats.at(this->current_chat)->getName() + "]"
            + " " + this->chats.at(this->current_chat)->getFileName()
            + " " + (this->chats.at(this->current_chat)->isSaved() ? "" : "*");
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

void App::saveCurrentChat(const std::string filename) {
    std::string fname;
    if (filename == "") {
        fname = chats.at(current_chat)->getFileName();
    } else {
        fname = filename;
        chats.at(current_chat)->setFileName(filename);
    }
    if (fname == "") {
        chats.at(current_chat)->log("No filename specified");
        return;
    }
    if (fname.find(".json") == std::string::npos) {
        fname += ".json";
    }
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
    std::string home = std::getenv("HOME");
    std::string path = std::filesystem::path(home) / CONFIG_DIR;
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
    auto file = std::ofstream(std::filesystem::path(path) / fname);
    file << json.dump(2);
    file.close();
    chats.at(current_chat)->markSaved();
}

int App::run() {
    while(!_exit.first) {
        auto maybeCmdStr = window.processInput();
        if(maybeCmdStr) {
            if (maybeCmdStr.value()[0] == '/') {
                auto cmdStr = maybeCmdStr.value().substr(1);
                auto cmd = cmdStr.substr(0, cmdStr.find(' '));
                auto argsIdx = cmdStr.find(' ');
                std::string args = "";
                if (argsIdx != std::string::npos) {
                    args = cmdStr.substr(argsIdx + 1);
                }
                if (!Command::run(cmd, this, args)) {
                    chats.at(current_chat)->log("Unknown command: " + cmd);
                }
                window.refreshStatus();
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