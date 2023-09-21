#pragma once

#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <atomic>

static std::atomic<unsigned int> next_id(1);

enum class Author {
    NONE,
    SYSTEM,
    USER,
    ASSISTANT
};

class Message {
public:
    // XXX: I cannot have const fields, because C++ is shitty and vector requires mutable fields
    unsigned int id = next_id++;
    time_t timestamp = time(nullptr);
    std::string message;
    Author author;

    Message(const std::string& message, Author author = Author::NONE);
    Message(const Message& msg);
    Message();
    ~Message();
};

class Chat {
protected:
    unsigned int id = next_id++;
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