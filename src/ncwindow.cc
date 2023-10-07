#include <ncurses.h>
#include <functional>
#include <optional>

#include "../config.h"
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

[[maybe_unused]]
void NCWindow::system(const std::string msg) {
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
    scrollok(w_chat, FALSE);
    scrollok(w_prompt, FALSE);
    fullRefresh();
}

void NCWindow::resize() {
    resizeterm(0, 0);
    width = getmaxx(stdscr);
    auto height = getmaxy(stdscr);
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
    printPrompt();
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
}

void NCWindow::updateLines(const std::shared_ptr<Chat> lChat) {
    auto messages = lChat->getMessages();
    unsigned int lastSeenIdx;
    if (lastMessageId == 0) {
        for(auto msg : messages) {
            addLines(msg);
            lastLine = lines.size();
        }
    } else {
        bool updated = false;
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
        if (updated) lastLine = lines.size();
    }
}

void NCWindow::printChat() {
    curs_set(0);
    wclear(w_chat);
    wrefresh(w_chat);
    unsigned int screenLines = LINES - 3;
    unsigned int startLine = lastLine > screenLines ? lastLine - screenLines : 0;
    for (unsigned int i = 0; i < screenLines && (i + startLine) < lines.size(); i++) {
        auto line = lines.at(i + startLine);
        wmove(w_chat, i, 0);
        wattrset(w_chat, line.attrs);
        wprintw(w_chat, "%s", line.text.c_str());
    }
    redraw();
    wmove(w_prompt, 0, prompt_pos);
    curs_set(1);
}

void NCWindow::feedback(const char ch) {
    inputChars++;
    prompt_pos++;
    printPrompt();
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

void NCWindow::printPrompt() {
    curs_set(0);
    wclear(w_prompt);
    wmove(w_prompt, 0, 0);
    auto visiblePrompt = inputBuffer.substr(SUB_MIN_0(prompt_pos, width-1), width - 1);
    wprintw(w_prompt, "%s", visiblePrompt.c_str());
    wmove(w_prompt, 0, prompt_pos - SUB_MIN_0(prompt_pos, width-1));
    curs_set(1);
    wrefresh(w_prompt);
}

void NCWindow::setChat(std::weak_ptr<Chat> chat) {
    this->chat = chat;
    rebuildLines();
    lastLine = lines.size();
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
            printPrompt();
        } else {
            flash();
        }
        break;
    case KEY_RIGHT:
        if (prompt_pos < inputChars) {
            prompt_pos++;
            printPrompt();
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
    case '\x05': // CTRL+E
    case KEY_END:
        prompt_pos = inputChars;
        printPrompt();
        break;
    case '\x01': // CTRL+A
    case KEY_HOME:
        prompt_pos = 0;
        printPrompt();
        break;
    case '\x17': { // CTRL+W
            if (prompt_pos > 0) {
                auto wordStart = inputBuffer.rfind(' ', prompt_pos - 1);
                if (wordStart == std::string::npos) wordStart = 0;
                inputBuffer.erase(wordStart, prompt_pos - wordStart);
                prompt_pos = wordStart;
                inputChars = inputBuffer.length();
                wclear(w_prompt);
                wprintw(w_prompt, "%s", inputBuffer.c_str());
                wmove(w_prompt, 0, prompt_pos);
                wrefresh(w_prompt);
            } else {
                flash();
            }
        }
        break;
    case '\x15': // CTRL+U
    case KEY_NPAGE: {
            auto scrollSize = (LINES - 3)/2;
            lastLine += scrollSize;
            if (lastLine > lines.size()) lastLine = lines.size();
            printChat();
        }
        break;
    case '\x04': // Ctrl+D
    case KEY_PPAGE: {
            auto scrollSize = (LINES - 3)/2;
            long minLastLine = (long)lines.size() < (long)LINES - 3 ? lines.size() - 1 : LINES - 4;
            lastLine = (long)lastLine - scrollSize < minLastLine ? minLastLine : lastLine - scrollSize;
            printChat();
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
        printPrompt();
        lastMessageId = lChat->getMessages().back().id;
    }
}

void NCWindow::refreshStatus() {
    printTitle();
    printStatus();
    printPrompt();
}
