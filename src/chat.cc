#include <string>
#include <functional>
#include <variant>
#include <algorithm>
#include "chat.hh"

Message::Message(const std::string &message, Author author) : message(message), author(author) {}
Message::Message(const Message &msg) :
    id(msg.id),
    timestamp(msg.timestamp),
    message(std::string(msg.message.begin(), msg.message.end())),
    author(msg.author) {}
Message::Message() : message(""), author(Author::NONE) {}
Message::~Message() {}

Chat::Chat(std::string name)
{
    this->name = name;
}

Chat::~Chat() {}

void Chat::addMsg(const Message msg)
{
    messages.push_back(msg);
    if (messages.size() > history_length) {
        messages.erase(messages.begin());
    }
}

std::vector<Message> Chat::getMessages() const
{
    return messages;
}

void Chat::addString(const std::string &str)
{
    addMsg({str, Author::NONE});
}

void Chat::send(const std::string &str, Author author)
{
    addMsg({str, author});
    // TODO: send to server
}

void Chat::onReceive(const std::string &str, Author author)
{
    addMsg({str, author});
}

std::string Chat::getName() const
{
    return name;
}