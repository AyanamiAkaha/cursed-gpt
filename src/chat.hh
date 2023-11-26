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
    std::string dateTime() const;

    Message(time_t timestamp, const std::string& message, Author author);
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

    std::vector<Message> messages;
    std::string name;
    std::string filename = "";
    bool saved = false;
    void addMsg(const Message msg);

    virtual bool isValidConfigKVP([[maybe_unused]] const std::string& key, [[maybe_unused]] const ConfigValue& value) const;
public:
    bool isTemplate = false;

    Chat(std::string name = "Chat");
    Chat(std::string name, std::vector<Message> template_messages);
    virtual ~Chat();
    void setConfigValue(const std::string& key, const ConfigValue& value);
    std::vector<Message> getMessages() const;
    void setMessages(const std::vector<Message>& msgs);
    void addString(const std::string& str);
    virtual void msg(const std::string& str, Author author = Author::USER);
    virtual void send();
    virtual void checkReceived();
    std::string getName() const;
    std::string getFileName() const;
    void setFileName(const std::string filename);
    void unsetFileName();
    void markSaved();
    bool isSaved() const;
    void log(const std::string& str);
    void systemPrompt(const std::string message);
    void impersonateAssistant(const std::string message);
};