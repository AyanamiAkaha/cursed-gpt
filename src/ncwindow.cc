#include <ncurses.h>
#include <functional>
#include <optional>

#include "ncwindow.hh"
#include "chat.hh"

void clean_curses() {
    if(!isendwin()) endwin();
}

NCWindow::NCWindow() {
    initCurses();
    width = getmaxx(stdscr);
    createWindows();
    atexit(clean_curses);
}

NCWindow::~NCWindow() {
    delwin(w_title);
    delwin(w_prompt);
    delwin(w_status);
    delwin(w_chat);
    clean_curses();
}

void NCWindow::log(std::string msg) {
    auto lChat = chat.lock();
    if(lChat != nullptr) {
        lChat.get()->addString(msg);
    } else {
        wclear(w_chat);
        wprintw(w_chat, "\n%s", msg.c_str());
        wrefresh(w_chat);
    }
}

void NCWindow::initCurses() {
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    use_default_colors();
    assume_default_colors(COLOR_WHITE, COLOR_BLACK);
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

void NCWindow::createWindows() {
    w_title = newwin(1, COLS, 0, 0);
    w_prompt = newwin(1, COLS, LINES - 1, 0);
    w_status = newwin(1, COLS, LINES - 2, 0);
    w_chat = newwin(LINES - 3, COLS, 1, 0);
    mvwchgat(w_chat, 0, 0, -1, A_NORMAL, 0, NULL);
    mvwchgat(w_prompt, 0, 0, -1, A_NORMAL, 0, NULL);
    scrollok(w_chat, TRUE);
    scrollok(w_prompt, TRUE);
    fullRefresh();
}

void NCWindow::resize() {
    resizeterm(0, 0);
    width = getmaxx(stdscr);
    auto height = getmaxy(stdscr);
    log("resize to " + std::to_string(width) + "x" + std::to_string(height));
    wresize(w_title, 1, width);
    wresize(w_prompt, 1, width);
    wresize(w_status, 1, width);
    wresize(w_chat, height - 3, width);
    mvwin(w_prompt, height - 1, 0);
    mvwin(w_status, height - 2, 0);
    mvwin(w_chat, 1, 0);
    mvwin(w_title, 0, 0);
    fullRefresh();
}

void NCWindow::fullRefresh() {
    wclear(w_prompt);
    wclear(w_status);
    wclear(w_chat);
    wclear(w_title);
    refresh();
    printTitle();
    printStatus();
    printChat();
    redraw();
}

void NCWindow::redraw() {
    refresh();
    wrefresh(w_title);
    wrefresh(w_status);
    wrefresh(w_chat);
    wrefresh(w_prompt);
}

void NCWindow::printChat() {
    curs_set(0);
    wclear(w_chat);
    wmove(w_chat, 0, 0);
    auto lChat = chat.lock();
    if (lChat != nullptr) {
        for(auto msg : lChat->getMessages()) {
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
            wprintw(w_chat, "\n%s", msg.message.c_str());
            wattroff(w_chat, COLOR_PAIR(1));
            wattroff(w_chat, COLOR_PAIR(2));
            wattroff(w_chat, COLOR_PAIR(3));
        }
    }
    redraw();
    wmove(w_prompt, 0, prompt_pos);
    curs_set(1);
}

void NCWindow::feedback(const char ch) {
    prompt_pos++;
    waddch(w_prompt, ch);
    wrefresh(w_prompt);
}

void NCWindow::clearPrompt() {
    prompt_pos = 0;
    wclear(w_prompt);
    wrefresh(w_prompt);
}

void NCWindow::setStatusCb(std::function<std::string()> getStatusText) {
    log("setStatusCb");
    this->getStatusText = getStatusText;
    printStatus();
}

void NCWindow::printStatus() {
    log("printStatus");
    auto status = getStatusText ? getStatusText() : "";
    wclear(w_status);
    mvwchgat(w_status, 0, 0, width, A_NORMAL, 4, NULL);
    wattrset(w_status, COLOR_PAIR(4));
    mvwprintw(w_status, 0, 1, "%s", status.c_str());
    wrefresh(w_status);
}

void NCWindow::printTitle() {
    log("printTitle");
    wclear(w_title);
    mvwchgat(w_title, 0, 0, width, A_BOLD, 4, NULL);
    wattrset(w_title, COLOR_PAIR(4));
    auto lChat = chat.lock();
    if (lChat != nullptr) {
        mvwprintw(w_title, 0, 1, "%s", lChat->getName().c_str());
    }
    wrefresh(w_title);
}

void NCWindow::setChat(std::weak_ptr<Chat> chat) {
    this->chat = chat;
    fullRefresh();
}

std::optional<std::string> NCWindow::processInput() {
    auto ch = getch();
    if (ch == ERR) return std::nullopt;
    if (ch == KEY_RESIZE) {
        resize();
        return std::nullopt;
    }
    if (ch == '\n') {
        std::optional<std::string> result = inputBuffer;
        inputBuffer.clear();
        clearPrompt();
        return result;
    } else {
        inputBuffer.push_back(ch);
        feedback(ch);
        return std::nullopt;
    }
}

void NCWindow::update() {
    auto lChat = chat.lock();
    if (lChat != nullptr
        && lChat->getMessages().size() > 0
        && lChat->getMessages().back().id != lastMessageId
    ) {
        printChat();
        lastMessageId = lChat->getMessages().back().id;
    }
}