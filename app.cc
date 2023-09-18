#include <iostream>
#include <memory>
#include <vector>
#include <ncurses.h>
#include "app.hh"
#include "chat.hh"

#define PROJECT_NAME "gpt-chat"

App::App(/* args */) {
    initCurses();
    w_title = newwin(1, COLS, 0, 0);
    w_prompt = newwin(1, COLS, LINES - 1, 0);
    w_status = newwin(1, COLS, LINES - 2, 0);
    w_chat = newwin(LINES - 3, COLS, 1, 0);
    newChat();
}

App::~App() {
    delwin(w_title);
    delwin(w_prompt);
    delwin(w_status);
    delwin(w_chat);
    endwin();
}

void App::initCurses() {
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    intrflush(stdscr, FALSE);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    clear();
}

CommandResult App::processInput() {
    auto ch = getch();
    if (ch == ERR) return NO_COMMAND;
    if (ch == '\n') {
        auto result = parseCommand(input_buffer);
        input_buffer.clear();
        wmove(w_prompt, 0, 0);
        wrefresh(w_prompt);
        return result;
    } else {
        input_buffer.push_back(ch);
        waddch(w_prompt, ch);
        wrefresh(w_prompt);
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

void App::printChat() {
    if (!needs_refresh) return;
    wclear(w_chat);
    wmove(w_chat, 0, 0);
    for(auto msg : chats.at(current_chat).getMessages()) {
        switch(msg.author) {
            case Author::SYSTEM:
                wattron(w_chat, COLOR_PAIR(1));
                break;
            case Author::USER:
                wattron(w_chat, COLOR_PAIR(2));
                break;
            case Author::ASSISTANT:
                wattron(w_chat, COLOR_PAIR(3));
                break;
            default:
                break;
        }
        wprintw(w_chat, msg.message.c_str());
        waddch(w_chat, '\n');
        wattroff(w_chat, COLOR_PAIR(1));
        wattroff(w_chat, COLOR_PAIR(2));
        wattroff(w_chat, COLOR_PAIR(3));
    }
    wrefresh(w_chat);
    refresh();
    needs_refresh = false;
}

void App::newChat() {
    chats.emplace_back();
}

int App::run() {
    mvwprintw(w_title, 0, 1, PROJECT_NAME);
    wrefresh(w_title);
    mvwprintw(w_status, 0, 1, "Chat %d/%d", current_chat + 1, chats.size());
    wrefresh(w_status);
    while(true) {
        auto result = processInput();
        if(result._exit) return result.exit_code;
        printChat();
    }
   return 0;
}