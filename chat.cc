#include <iostream>
#include <functional>
#include <variant>
#include "chat.hh"

Chat::Chat() {}

Chat::~Chat() {}

void Chat::addMsg(const Message msg) {
    messages.push_back(msg);
    if (messages.size() > history_length) {
        messages.erase(messages.begin());
    }
}

std::vector<Message> Chat::getMessages() const {
    return messages;
}

void Chat::addString(const std::string &str) {
    addMsg({str, Author::NONE});
}

void Chat::send(const std::string &str, Author author) {
    addMsg({str, author});
    // TODO: send to server
}

void Chat::onReceive(const std::string &str, Author author) {
    addMsg({str, author});
}