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

std::string sanitizeFileName(const std::string& input) {
    std::string sanitized = input;
    std::replace_if(sanitized.begin(), sanitized.end(), [](char c) {
        return !(isalnum(c) || c == '.' || c == '_' || c == '-');
    }, '_');
    const size_t maxFileNameLength = 255;
    if (sanitized.length() > maxFileNameLength) {
        sanitized.resize(maxFileNameLength);
    }
    return sanitized;
}

std::map<std::string, std::function<void(App*, std::string)>> Command::commands;

App::App(/* args */) {
    newChat("");
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

void App::cmdNew(std::string args) {
    // split args by spaces, trim all
    std::vector<std::string> argsList;
    std::string arg;
    for (auto c: args) {
        if (c == ' ') {
            if (arg != "") {
                argsList.push_back(arg);
                arg = "";
            }
        } else {
            arg += c;
        }
    }
    if(arg != "") {
        argsList.push_back(arg);
    }
    if (argsList.size() == 0) {
        newChat("");
    } else if (argsList.size() == 1) {
        loadTemplate(argsList.at(0));
        if (argsList.at(0) == "template") {
            currentChat().log("If you want to create new template use: /new template <template name>");
        }
    } else if (argsList.size() == 2 && argsList.at(0) == "template") {
        newTemplate(argsList.at(1));
    } else {
        currentChat().log("Usage: /new [template <template name>]");
    }
}

void App::newChat(const std::string name) {
    auto chatName = name;
    if (name == "") {
        chatName = PROJECT_NAME + std::to_string(chats.size());
    }
    auto chat = std::make_shared<GptChat>(chatName);
    chats.push_back(chat);
    current_chat = chats.size() - 1;
    window.setChat(chat);
}

void App::newTemplate(std::string args) {
    std::string name = sanitizeFileName(args);
    std::string fname = name + ".json";
    if(name == "") {
        currentChat().log("Usage: /new <template name>");
        return;
    }
    std::string path = std::filesystem::path(std::getenv("HOME")) / "." PROJECT_NAME / "templates" / fname;
    if(std::filesystem::exists(path)) {
        currentChat().log("Template '" + fname + "' already exists. Use different name or delete it manually first.");
        return;
    } else {
        auto chat = std::make_shared<Chat>(name);
        chat->isTemplate = true;
        chat->setFileName(fname);
        chats.push_back(chat);
        current_chat = chats.size() - 1;
        window.setChat(chat);
    }
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
    if (currentChat().isTemplate) {
        path = std::filesystem::path(home) /  "." PROJECT_NAME / "templates";
    }
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
    auto file = std::ofstream(std::filesystem::path(path) / fname);
    file << json.dump(2);
    file.close();
    currentChat().markSaved();
}

void App::loadChat(std::string filename) {
    load(filename, std::filesystem::path(std::getenv("HOME")) / "." PROJECT_NAME);
}

void App::loadTemplate(std::string filename) {
    load(filename, std::filesystem::path(std::getenv("HOME")) / "." PROJECT_NAME / "templates");
    currentChat().unsetFileName();
}

void App::load(const std::string filename, std::string rootPath) {
    std::string fname;
    if (filename == "") {
        currentChat().log("No filename specified");
        return;
    }
    if (filename.find(".json") == std::string::npos) {
        fname = filename + ".json";
    } else {
        fname = filename;
    }
    auto path = rootPath;
    auto file = std::ifstream(std::filesystem::path(path) / fname);
    if (!file.is_open()) {
        currentChat().log("Cannot open chat '" + fname + "': " + strerror(errno));
        return;
    }
    std::vector<Message> messages;
    try {
        auto json = nlohmann::json::parse(file);
        file.close();
        auto msgs = json["messages"];
        for (auto msg: msgs) {
            messages.emplace_back(Message{
                msg["timestamp"],
                msg["message"],
                (Author)msg["author"]
            });
        }
    } catch (const nlohmann::json::exception &e) {
        currentChat().log("Cannot parse chat '" + fname + "': " + e.what());
        return;
    }
    newChat(filename);
    currentChat().setFileName(fname);
    currentChat().setMessages(messages);
}

void App::exportCurrentChat(std::string filename) {
    std::string fname;
    if (filename == "") {
        fname = currentChat().getFileName();
    } else {
        fname = filename;
    }
    if (fname == "") {
        currentChat().log("No filename specified");
        return;
    }
    if (fname.find(".txt") == std::string::npos) {
        fname += ".txt";
    }
    if (fname.find(".json") != std::string::npos) {
        fname.replace(fname.find(".json"), 5, "");
    }
    auto msgs = currentChat().getMessages();
    if (fname.find("~") != std::string::npos) {
        std::string home = std::getenv("HOME");
        fname.replace(fname.find("~"), 1, home);
    }
    auto file = std::ofstream(fname);
    for (auto msg: msgs) {
        switch (msg.author) {
            case Author::SYSTEM:
                file << "[" << msg.dateTime() << "] ## " << msg.message << std::endl;
                break;
            case Author::USER:
                file << "[" << msg.dateTime() << "] " << msg.message << std::endl;
                break;
            case Author::ASSISTANT:
                file << "[" << msg.dateTime() << "] >> " << msg.message << std::endl;
                break;
            case Author::NONE:
                file << "[" << msg.dateTime() << "] ** " << msg.message << std::endl;
                break;
            default:
                file << "[" << msg.dateTime() << "] " << "(\?\?\?)" << msg.message << std::endl;
        }
    }
    file.close();
    auto fullPath = std::filesystem::absolute(fname);
    currentChat().log("Chat exported to " + fullPath.string());
}

void App::cmdList(std::string args) {
    if (args == "") {
        currentChat().log("Usage: /list <chats|saved|templates>");
        return;
    }
    if (args == "chats") {
        listChats();
    } else if (args == "templates") {
        listTemplates();
    } else if (args == "saved") {
        listSavedChats();
    } else {
        currentChat().log("Usage: /list <chats|saved|templates>");
    }
}

void App::listSavedChats() {
    std::string home = std::getenv("HOME");
    std::string path = std::filesystem::path(home) /  "." PROJECT_NAME;
    currentChat().log("Saved chats:");
    for (auto& p: std::filesystem::directory_iterator(path)) {
        if (p.path().extension() == ".json") {
            currentChat().log(p.path().filename());
        }
    }
}

void App::listTemplates() {
    currentChat().log("Templates:");
    // TODO: list templates
    /*
    for (auto& p: std::filesystem::directory_iterator(TEMPLATES_DIR)) {
        if (p.path().extension() == ".json") {
            currentChat().log(p.path().filename());
        }
    }
    */
}

void App::listChats() {
    currentChat().log("Chats:");
    for (auto& chat: chats) {
        currentChat().log(chat->getName() + " (" + chat->getFileName() +")");
    }
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
                currentChat().msg(maybeCmdStr.value(), Author::USER);
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
        } catch (const std::invalid_argument& e) {
            currentChat().log("Invalid temperature: " + args + ": " + e.what());
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