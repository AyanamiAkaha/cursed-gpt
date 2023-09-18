#include <iostream>
#include <functional>
#include <variant>
#include "chat.hh"

Chat::Chat()
{
}

Chat::~Chat()
{
}

void Chat::addMsg(std::string msg, Author author){
    auto message = Message(msg, author);
    messages.push_back(message);
}

/**
 * @brief Redraws chat window
 */
void Chat::redraw() {
    // clear screen
    std::cout << "\033[2J\033[1;1H";
    // print messages
    for(auto message : messages) {
        std::cout << message;
    }
}

Message::Message(std::string message, Author author)
    : message(std::make_shared<std::string>(std::move(message))),
        author(author) {}

Message::~Message() {}

std::string Message::printAuthor() const {
    switch (author) {
    case Author::SYSTEM:
        return "[SYSTEM]\n";
    case Author::USER:
        return "[USER]\n";
    case Author::ASSISTANT:
        return "[ASSISTANT]\n";
    default:
        throw std::runtime_error("Unknown author. This should never happen.");
    }
}

std::string Message::asString() const {
    return printAuthor() + *message + "\n\n";
}

std::ostream &operator<<(std::ostream &os, const Message &msg) {
    os << msg.asString();
    return os;
}