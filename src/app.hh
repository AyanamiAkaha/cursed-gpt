#pragma once

#include <vector>
#include <map>
#include <optional>
#include <signal.h>

#include "ncwindow.hh"

class Chat;

class App
{
private:
    std::vector<std::shared_ptr<Chat>> chats;
    int current_chat = 0;
    NCWindow window;
    std::pair<bool, int> _exit = { false, 0 };

    void listSavedChats();
    void listTemplates();
    void listChats();
    void load(std::string filename, std::string rootPath);
public:
    App();
    ~App();
    int run();
    void quit(int exit_code);
    void newChat(const std::string name);
    void nextChat();
    void prevChat();
    void chatNum(int num);
    Chat& currentChat() const;
    void setTemperature(std::string args);
    void setModel(std::string args);
    void saveCurrentChat(std::string filename);
    void loadChat(std::string filename);
    void loadTemplate(std::string filename);
    void exportCurrentChat(std::string path);
    void cmdList(std::string args);
    void cmdNew(std::string args);
    void newTemplate(std::string args);
};

class Command {
protected:
    using CommandFn = std::function<void(App*, std::string)>;
    static std::map<std::string, CommandFn> commands;
public:
    static bool run(std::string name, App* app, std::string args) {
        auto it = commands.find(name);
        if (it != commands.end()) {
            CommandFn& cmd = it->second;
            cmd(app, args);
            return true;
        }
        return false;
    }
    Command(std::string name, CommandFn code) {
        commands[name] = code;
    }
    virtual ~Command() = default;
};


#define COMMAND(name, code) \
    static void _##name(App* app, [[maybe_unused]] std::string args) { code; } \
    static Command _##name##_instance(#name, _##name);
