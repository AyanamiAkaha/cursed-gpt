#pragma once

#include <vector>
#include <string>
#include <variant>
#include <functional>
#include <memory>

struct AuthorSystem { const std::string name = "system"; };
struct AuthorUser { const std::string name = "user"; };
struct AuthorAssistant { const std::string name = "assistant"; };

using Author = std::variant<AuthorSystem, AuthorUser, AuthorAssistant>;

class Message {
private:
    std::shared_ptr<std::string> message;
    Author author;
public:
    Message(std::string msg, Author author);
    ~Message();
};

class Chat
{
private:
    std::vector<Message> messages;
public:
    Chat();
    ~Chat();
    void sendMsg(std::string msg, Author author);
};