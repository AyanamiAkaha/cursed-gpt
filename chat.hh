#pragma once

#include <vector>
#include <string>
#include <memory>


enum class Author {
    SYSTEM,
    USER,
    ASSISTANT
};

class Message {
private:
    std::shared_ptr<std::string> message;
    Author author;
    std::string printAuthor() const;
public:
    Message(std::string msg, Author author);
    ~Message();
    std::string asString() const;
    friend std::ostream &operator<<(std::ostream &os, const Message &msg);
};

class Chat
{
private:
    std::vector<Message> messages;
    /// @brief Number of starting lines that are never deleted from history
    uint32_t static_lines = 0;
    /// @brief Maximum number of lines in chat history
    uint32_t max_lines = 10;
public:
    Chat();
    ~Chat();
    void addMsg(std::string msg, Author author);
    void redraw();
};