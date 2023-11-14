#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <ncurses.h>
#include <map>
#include <fstream>
#include <json.hpp>
#include <filesystem>

#include "../config.h"
#include "app.hh"
#include "chat.hh"
#include "ncwindow.hh"
#include "gpt_chat.hh"

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

Chat& App::currentChat() const {
    return *chats.at(current_chat);
}

void App::saveCurrentChat(const std::string filename) {
    std::string fname;
    if (filename == "") {
        fname = currentChat().getFileName();
    } else {
        fname = filename;
        currentChat().setFileName(filename);
    }
    if (fname == "") {
        currentChat().log("No filename specified");
        return;
    }
    if (fname.find(".json") == std::string::npos) {
        fname += ".json";
    }
    auto msgs = currentChat().getMessages();
    auto json = nlohmann::json::object();
    json["template"] = currentChat().getName();
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
    std::string path = std::filesystem::path(home) /  "." PROJECT_NAME;
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
    auto file = std::ofstream(std::filesystem::path(path) / fname);
    file << json.dump(2);
    file.close();
    currentChat().markSaved();
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
                    currentChat().log("Unknown command: " + cmd);
                }
                window.refreshStatus();
            } else {
                currentChat().send(maybeCmdStr.value(), Author::USER);
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

void App::setTemperature(std::string args) {
    if (args == "") {
        currentChat().log("Usage: /temperature <value>");
    } else {
        try {
            currentChat().setConfigValue("temperature", std::stod(args));
            currentChat().log("Temperature changed to " + args);
        } catch (std::invalid_argument) {
            currentChat().log("Invalid temperature: " + args);
        }
    }
}

void App::setModel(std::string args) {
    if (args == "") {
        currentChat().log("Usage: /model <model name>");
    } else {
        currentChat().setConfigValue("model", args);
        currentChat().log("Model changed to " + args);
    }
}