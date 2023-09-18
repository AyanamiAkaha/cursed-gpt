#include <ncurses.h>

// TEMP
#include <iostream>

#include "ncwindow.hh"
#include "chat.hh"

NCWindow::NCWindow() {
    initCurses();
    w_title = newwin(1, COLS, 0, 0);
    w_prompt = newwin(1, COLS, LINES - 1, 0);
    w_status = newwin(1, COLS, LINES - 2, 0);
    w_chat = newwin(LINES - 3, COLS, 1, 0);
    refresh();
}

NCWindow::~NCWindow() {
    delwin(w_title);
    delwin(w_prompt);
    delwin(w_status);
    delwin(w_chat);
    endwin();
}

void NCWindow::initCurses() {
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
    init_pair(4, COLOR_WHITE, COLOR_MAGENTA);
    clear();
}

void NCWindow::redraw() {
    wrefresh(w_title);
    wrefresh(w_status);
    wrefresh(w_chat);
    refresh();
    wrefresh(w_prompt);
}

void NCWindow::printChat(const Chat& chat) {
    curs_set(0);
    wclear(w_chat);

    wmove(w_chat, 0, 0);
    for(auto msg : chat.getMessages()) {
        switch(msg.author) {
            case Author::SYSTEM:
            case Author::NONE:
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
    redraw();
    wmove(w_prompt, 0, prompt_pos);
    curs_set(1);
}

void NCWindow::feedback(char ch) {
    prompt_pos++;
    waddch(w_prompt, ch);
    wrefresh(w_prompt);
}

void NCWindow::clearPrompt() {
    prompt_pos = 0;
    wclear(w_prompt);
    wrefresh(w_prompt);
}

void NCWindow::setStatus(std::string status) {
    // wclear(w_status);
    mvwchgat(w_status, 0, 0, -1, A_NORMAL, 4, NULL);
    wattrset(w_status, COLOR_PAIR(4));
    mvwprintw(w_status, 0, 1, status.c_str());
    wrefresh(w_status);
}

void NCWindow::setTitle(std::string title) {
    // wclear(w_title);
    mvwchgat(w_title, 0, 0, -1, A_BOLD, 4, NULL);
    wattrset(w_title, COLOR_PAIR(4));
    mvwprintw(w_title, 0, 1, title.c_str());
    wrefresh(w_title);
}