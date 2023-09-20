#pragma once

#include <vector>
#include <string>
#include <memory>

static unsigned int next_id = 1;

enum class Author {
    NONE,
    SYSTEM,
    USER,
    ASSISTANT
};

struct Message {
    std::string message;
    Author author;
    time_t timestamp = time(nullptr);
    unsigned int id = next_id++;
};

class Chat {
private:
    std::vector<Message> template_messages;
    std::vector<Message> messages;
    uint32_t history_length = 100;
    std::string name;
    void addMsg(const Message msg);
public:
    Chat(std::string name = "Chat");
    virtual ~Chat();
    std::vector<Message> getMessages() const;
    void addString(const std::string& str);
    virtual void send(const std::string& str, Author author = Author::USER);
    virtual void onReceive(const std::string& str, Author author = Author::ASSISTANT);
    std::string getName() const;
};