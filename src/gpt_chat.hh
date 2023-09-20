#pragma once

#include "chat.hh"

class ApiRequest {
    Message message;
public:
    ApiRequest(const Message& message);
    ApiRequest(const ApiRequest& req);
    ~ApiRequest();
    std::string getJson() const;
};

class GptChat : public Chat {
private:
    // httplib::Client client;
    // nlohmann::json json;
public:
    GptChat(std::string name = "GptChat");
    ~GptChat();
    void send(const std::string& str, Author author = Author::USER);
    void onReceive(const std::string& str, Author author = Author::ASSISTANT);
};