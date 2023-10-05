#pragma once

#include <memory>
#include <string>
#include <functional>
#include <optional>
#include <ncurses.h>

class Chat;
class Message;
enum class Author;

struct ScreenLine {
    attr_t attrs;
    std::string text;
};

class NCWindow {
private:
    WINDOW* w_prompt;
    WINDOW* w_chat;
    WINDOW* w_title;
    WINDOW* w_status;
    std::string inputBuffer;
    unsigned int prompt_pos = 0;
    unsigned int inputChars = 0;
    unsigned int lastLine = 0;
    std::vector<ScreenLine> lines;
    unsigned int width = 0;
    std::weak_ptr<Chat> chat;
    std::function<std::string()> getStatusText;
    unsigned int lastMessageId = 0;

    void log(const std::string msg);
    void initCurses();
    void createWindows();
    void printTitle();
    void printStatus();
    void fullRefresh();
    void feedback(const char);
    void resize();
    void redraw();
    void clearPrompt();
    void rebuildLines();
    void updateLines(const std::shared_ptr<Chat> lChat);
    void addLines(const Message& message);
    void printChat();
    attr_t author2attr(const Author author) const;
public:
    NCWindow();
    ~NCWindow();
    void setStatusCb(std::function<std::string()> getStatusText);
    void setChat(std::weak_ptr<Chat> chat);
    std::optional<std::string> processInput();
    void update();
    void refreshStatus();
};