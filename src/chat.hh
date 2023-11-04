#pragma once

#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <variant>

using ConfigValue = std::variant<std::string, double>;

enum class Author {
    NONE,
    SYSTEM,
    USER,
    ASSISTANT
};

class Message {
public:
    static std::atomic<unsigned int> next_id;
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
private:
    static std::atomic<unsigned int> next_id;
protected:
    unsigned int id = next_id++;
    std::unordered_map<std::string, ConfigValue> config;

    std::vector<Message> template_messages;
    std::vector<Message> messages;
    uint32_t history_length = 100;
    std::string name;
    std::string filename = "";
    bool saved = false;
    void addMsg(const Message msg);

    virtual bool isValidConfigKVP(const std::string& key, const ConfigValue& value);
public:
    Chat(std::string name = "Chat");
    Chat(std::string name, std::vector<Message> template_messages);
    virtual ~Chat();
    void setConfigValue(const std::string& key, const ConfigValue& value);
    std::vector<Message> getMessages() const;
    void addString(const std::string& str);
    virtual void send(const std::string& str, Author author = Author::USER);
    virtual void checkReceived();
    std::string getName() const;
    std::string getFileName() const;
    void setFileName(const std::string filename);
    void markSaved();
    bool isSaved() const;
    void log(const std::string& str);
};