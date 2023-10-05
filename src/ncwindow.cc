#include <ncurses.h>
#include <functional>
#include <optional>

#include "../config.h"
#include "ncwindow.hh"
#include "chat.hh"

#ifdef FILE_LOG
#include <cstdio>
#endif

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

void NCWindow::log(const std::string msg) {
#ifndef FILE_LOG
    auto lChat = chat.lock();
    if(lChat != nullptr) {
        lChat.get()->addString(msg);
    } else {
        wclear(w_chat);
        wprintw(w_chat, "\n%s", msg.c_str());
        wrefresh(w_chat);
    }
#else 
#warning "FILE_LOG is enabled"
    auto LOG_FILE = fopen("/tmp/" PROJECT_NAME ".log", "a");
    fprintf(LOG_FILE, "%s\n", msg.c_str());
    fclose(LOG_FILE);
#endif
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
    nodelay(stdscr, FALSE);
    halfdelay(20);
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
    rebuildLines();
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

attr_t NCWindow::author2attr(const Author author) const {
    switch(author) {
        case Author::SYSTEM:
        case Author::NONE:
            return COLOR_PAIR(1);
        case Author::USER:
            return COLOR_PAIR(2) | A_BOLD;
        case Author::ASSISTANT:
            return COLOR_PAIR(3);
        default:
            return A_NORMAL;
    }
}

void NCWindow::rebuildLines() {
    lines.clear();
    auto lChat = chat.lock();
    if (lChat != nullptr) {
        auto messages = lChat->getMessages();
        for(auto msg : messages) {
            addLines(msg);
        }
    }
}

void NCWindow::addLines(const Message& message) {
    std::string l;
    for(unsigned int i=0,j=0; i<message.message.length(); i++, j++) {
        if (message.message[i] != '\n') {
            l.push_back(message.message[i]);
        }
        if (message.message[i] == '\n' || j == width - 1 || i == message.message.length() - 1) {
            ScreenLine line;
            line.attrs = author2attr(message.author);
            line.text = l;
            lines.push_back(line);
            j = 0;
            l.clear();
        }
    }
    // DEBUG - log lines
    log("-----------------");
    log("addLines(" + std::to_string(message.id) + ")");
    for(auto line : lines) {
        log("\"" + line.text + "\"");
    }
    log("lines.size() = " + std::to_string(lines.size()));
}

void NCWindow::updateLines(const std::shared_ptr<Chat> lChat) {
    bool updated = false;
    auto messages = lChat->getMessages();
    unsigned int lastSeenIdx;
    if (lastMessageId == 0) {
        for(auto msg : messages) {
            addLines(msg);
        }
        updated = true;
    } else {
        for (int i = messages.size() - 1; i >= 0; i--) {
            if (messages.at(i).id == lastMessageId) {
                lastSeenIdx = i;
                break;
            }
        }
        for (unsigned int i = lastSeenIdx + 1; i < messages.size(); i++) {
            addLines(messages[i]);
            updated = true;
        }
    }
    // DEBVG log messages and lines
    if (updated) {
        log("-----------------");
        log("updateLines\n");
        log("messages: ");
        for (auto message : messages) {
            if (message.id == lastMessageId) {
                break;
            }
            log(message.message);
        }
        log("messages.size() = " + std::to_string(messages.size()));
        log("lines: ");
        for(auto line : lines) {
            log("\"" + line.text + "\"");
        }
        log("lines.size() = " + std::to_string(lines.size()));
    }
}

void NCWindow::printChat() {
    curs_set(0);
    wclear(w_chat);
    wrefresh(w_chat);
    unsigned int screenLines = LINES - 3;
    unsigned int totalLines = lines.size();
    unsigned int startLine = totalLines > screenLines ? totalLines - screenLines : 0;
    for (unsigned int i = startLine; i < lines.size(); i++) {
        auto line = lines[i];
        wmove(w_chat, i - startLine, 0);
        wattrset(w_chat, line.attrs);
        wprintw(w_chat, "%s", line.text.c_str());
    }
    redraw();
    wmove(w_prompt, 0, prompt_pos);
    curs_set(1);
}

void NCWindow::feedback(const char ch) {
    prompt_pos++;
    inputChars++;
    winsch(w_prompt, ch);
    wmove(w_prompt, 0, prompt_pos);
    wrefresh(w_prompt);
}

void NCWindow::clearPrompt() {
    prompt_pos = 0;
    inputChars = 0;
    wclear(w_prompt);
    wrefresh(w_prompt);
}

void NCWindow::setStatusCb(std::function<std::string()> getStatusText) {
    this->getStatusText = getStatusText;
    printStatus();
}

void NCWindow::printStatus() {
    curs_set(0);
    auto status = getStatusText ? getStatusText() : "";
    wclear(w_status);
    mvwchgat(w_status, 0, 0, width, A_NORMAL, 4, NULL);
    wattrset(w_status, COLOR_PAIR(4));
    mvwprintw(w_status, 0, 1, "%s", status.c_str());
    wmove(w_prompt, 0, prompt_pos);
    curs_set(1);
    wrefresh(w_status);
    wrefresh(w_prompt);
}

void NCWindow::printTitle() {
    curs_set(0);
    wclear(w_title);
    mvwchgat(w_title, 0, 0, width, A_BOLD, 4, NULL);
    wattrset(w_title, COLOR_PAIR(4));
    auto lChat = chat.lock();
    if (lChat != nullptr) {
        mvwprintw(w_title, 0, 1, "%s", lChat->getName().c_str());
    }
    wmove(w_prompt, 0, prompt_pos);
    curs_set(1);
    wrefresh(w_title);
    wrefresh(w_prompt);
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
    switch(ch) {
    case KEY_BACKSPACE:
    case 127:
    case '\b':
        if (prompt_pos > 0) {
            prompt_pos--;
            inputChars--;
            wmove(w_prompt, 0, prompt_pos);
            wdelch(w_prompt);
            inputBuffer.erase(prompt_pos, 1);
        } else {
            flash();
        }
        wrefresh(w_prompt);
        break;
    case KEY_LEFT:
        if (prompt_pos > 0) {
            prompt_pos--;
            wmove(w_prompt, 0, prompt_pos);
            wrefresh(w_prompt);
        } else {
            flash();
        }
        break;
    case KEY_RIGHT:
        if (prompt_pos < inputChars) {
            prompt_pos++;
            wmove(w_prompt, 0, prompt_pos);
            wrefresh(w_prompt);
        } else {
            flash();
        }
        break;
    case KEY_DC:
        if (prompt_pos < inputChars) {
            wdelch(w_prompt);
            inputBuffer.erase(prompt_pos, 1);
            inputChars--;
            wrefresh(w_prompt);
        } else {
            flash();
        }
        break;
    case KEY_NPAGE: {
            auto scrollSize = (LINES - 3)/2;
            lastLine = (lastLine - (LINES - 3)/2) % lines.size();
            printChat();
            wrefresh(w_chat);
        }
        break;
    case KEY_PPAGE: {
            auto scrollSize = (LINES - 3)/2;
            lastLine = (lastLine + (LINES - 3)/2) % lines.size();
            printChat();
            wrefresh(w_chat);
        }
        break;
    case '\n': {
        std::optional<std::string> result = inputBuffer;
        inputBuffer.clear();
        clearPrompt();
        return result;
    }
    default:
        inputBuffer.insert(prompt_pos, 1, ch);
        feedback(ch);
    }
    return std::nullopt;
}

void NCWindow::update() {
    auto lChat = chat.lock();
    if (lChat != nullptr
        && lChat->getMessages().size() > 0
        && lChat->getMessages().back().id != lastMessageId
    ) {
        printTitle();
        printStatus();
        updateLines(lChat);
        printChat();
        lastMessageId = lChat->getMessages().back().id;
    }
}

void NCWindow::refreshStatus() {
    printTitle();
    printStatus();
}
