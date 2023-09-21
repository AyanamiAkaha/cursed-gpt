#pragma once

#include <atomic>
#include <string>

#include "concurrent_queue.hh"
#include "chat.hh"

class ApiRequest {
    unsigned int chatId;
    std::unique_ptr<char[]> body;
public:
    ApiRequest(unsigned int chatId, std::unique_ptr<char[]>&& body);
    ApiRequest(const ApiRequest& req);
    ApiRequest(const std::vector<Message>& messages);
    ~ApiRequest() = default;
};

class GptChat : public Chat {
private:
    httplib::Client client;
    ConcurrentQueue<ApiRequest> reqQueue;
    ConcurrentQueue<ApiRequest> resQueue;
public:
    GptChat(std::string name = "GptChat");
    ~GptChat();
    void send(const std::string& str, Author author = Author::USER);
    void onReceive(const std::string& str, Author author = Author::ASSISTANT);
};