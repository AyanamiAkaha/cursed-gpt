#pragma once

#include <memory>
#include <string>
#include <functional>
#include <optional>

class Chat;
class Message;

// WINDOW type needed by ncurses
struct _win_st;
typedef struct _win_st WINDOW;

class NCWindow {
private:
    WINDOW* w_prompt;
    WINDOW* w_chat;
    WINDOW* w_title;
    WINDOW* w_status;
    std::string inputBuffer;
    int prompt_pos = 0;
    int width = 0;
    std::weak_ptr<Chat> chat;
    std::function<std::string()> getStatusText;
    unsigned int lastMessageId = 0;

    void log(std::string msg);
    void initCurses();
    void createWindows();
    void printTitle();
    void printStatus();
    void fullRefresh();
    void feedback(const char);
    void resize();
    void redraw();
    void clearPrompt();
    void printChat();
public:
    NCWindow();
    ~NCWindow();
    void setStatusCb(std::function<std::string()> getStatusText);
    void setChat(std::weak_ptr<Chat> chat);
    std::optional<std::string> processInput();
    void update();
    void refreshStatus();
};