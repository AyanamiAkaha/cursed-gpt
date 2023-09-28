#include <string>
#include <functional>
#include <variant>
#include <algorithm>
#include "chat.hh"

std::atomic<unsigned int> Message::next_id(1);
Message::Message(const std::string &message, Author author) : message(message), author(author) {}
Message::Message(const Message &msg) :
    id(msg.id),
    timestamp(msg.timestamp),
    message(std::string(msg.message.begin(), msg.message.end())),
    author(msg.author) {}
Message::Message() : message(""), author(Author::NONE) {}
Message::~Message() {}

std::atomic<unsigned int> Chat::next_id(1);

Chat::Chat(std::string name) : name(name) {}
Chat::Chat(std::string name, std::vector<Message> template_messages) : template_messages(template_messages), name(name) {}
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

void Chat::checkReceived() {}

std::string Chat::getName() const {
    return name;
}

void Chat::log(const std::string &str) {
    addString(str);
}