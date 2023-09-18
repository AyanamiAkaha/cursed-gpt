#include <functional>
#include <variant>
#include "chat.hh"

std::string authorName(Author& author) {
    return std::visit([](const auto& type) { return type.name; }, author);
}

Chat::Chat()
{
}

Chat::~Chat()
{
}

void Chat::sendMsg(std::string msg, Author author){
    auto message = Message(msg, author);
    messages.push_back(message);
}

Message::Message(std::string message, Author author)
    : message(std::make_shared<std::string>(std::move(message))),
        author(std::move(author)) {}

Message::~Message() {}