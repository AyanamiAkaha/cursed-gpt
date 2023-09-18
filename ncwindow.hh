#pragma once

#include <string>

class Chat;

// WINDOW type needed by ncurses
struct _win_st;
typedef struct _win_st WINDOW;

class NCWindow {
private:
    WINDOW* w_prompt;
    WINDOW* w_chat;
    WINDOW* w_title;
    WINDOW* w_status;
    int prompt_pos = 0;

    void initCurses();
public:
    NCWindow();
    ~NCWindow();
    void resize();
    void redraw();
    void feedback(char);
    void clearPrompt();
    void printChat(const Chat& chat);
    void setTitle(std::string title);
    void setStatus(std::string status);
};