#include <string>
#include <functional>
#include <variant>
#include <algorithm>
#include <ctime>
#include "chat.hh"

using ConfigValue = std::variant<std::string, double>;

std::atomic<unsigned int> Message::next_id(1);

Message::Message(const Message &msg) :
    id(msg.id),
    timestamp(msg.timestamp),
    message(std::string(msg.message.begin(), msg.message.end())),
    author(msg.author) {}
Message::Message(time_t timestamp, const std::string &message, Author author) :
    timestamp(timestamp),
    message(message),
    author(author) { id = next_id++; }
Message::Message(const std::string &message, Author author) : Message(time(nullptr), message, author) {}
Message::Message() : Message("") {}
Message::~Message() {}

std::string Message::dateTime() const {
    char buf[80];
    struct tm *timeinfo = gmtime(&timestamp);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    return std::string(buf);
}

std::atomic<unsigned int> Chat::next_id(1);

Chat::Chat(std::string name) : name(name) {}
Chat::~Chat() {}

bool Chat::isValidConfigKVP([[maybe_unused]] const std::string &key, [[maybe_unused]] const ConfigValue &value) const {
    return false;
}

void Chat::setConfigValue(const std::string &key, const ConfigValue &value) {
    if (isValidConfigKVP(key, value)) {
        config[key] = value;
    }
}

void Chat::addMsg(const Message msg)
{
    messages.push_back(msg);
    saved = false;
}

std::vector<Message> Chat::getMessages() const
{
    return messages;
}

void Chat::setMessages(const std::vector<Message> &msgs)
{
    messages = msgs;
}

void Chat::addString(const std::string &str)
{
    addMsg({str, Author::NONE});
}

void Chat::msg(const std::string &str, Author author)
{
    addMsg({str, author});
    send();
}

void Chat::systemPrompt(const std::string message) {
    addMsg({message, Author::SYSTEM});
    send();
}

void Chat::impersonateAssistant(const std::string message) {
    addMsg({message, Author::ASSISTANT});
    send();
}

void Chat::send() {/* dummy */}

void Chat::checkReceived() {}

std::string Chat::getName() const {
    return name;
}

void Chat::log(const std::string &str) {
    addString(str);
}

std::string Chat::getFileName() const {
    return filename;
}

void Chat::setFileName(const std::string filename) {
    filename.find(".json") == std::string::npos
        ? this->filename = filename + ".json"
        : this->filename = filename;
}

void Chat::unsetFileName() {
    filename = "";
}

void Chat::markSaved() {
    saved = true;
}

bool Chat::isSaved() const {
    return saved;
}