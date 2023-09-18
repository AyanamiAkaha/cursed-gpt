#pragma once

#include <vector>
#include <string>
#include <memory>


enum class Author {
    NONE,
    SYSTEM,
    USER,
    ASSISTANT
};

struct Message {
    std::string message;
    Author author;
};

class Chat {
private:
    std::vector<Message> template_messages;
    std::vector<Message> messages;
    uint32_t history_length = 100;
    void addMsg(const Message msg);
public:
    Chat();
    ~Chat();
    std::vector<Message> getMessages() const;
    void addString(const std::string& str);
    void send(const std::string& str, Author author = Author::USER);
    void onReceive(const std::string& str, Author author = Author::ASSISTANT);
};