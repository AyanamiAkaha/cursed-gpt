#pragma once

#include <atomic>
#include <string>

#include "concurrent_queue.hh"
#include "chat.hh"

static std::atomic<unsigned int> next_id(1);

class ApiRequest {
    unsigned int id = next_id++;
    std::unique_ptr<char[]> body;
public:
    ApiRequest(std::unique_ptr<char[]>&& body);
    ApiRequest(const ApiRequest& req);
    ApiRequest(const std::vector<Message>& messages);
    ~ApiRequest() = default;
};

class GptChat : public Chat {
private:
    httplib::Client client;
    ConcurrentQueue<ApiRequest> reqQueue;
    ConcurrentQueue<std::string> resQueue;
public:
    GptChat(std::string name = "GptChat");
    ~GptChat();
    void send(const std::string& str, Author author = Author::USER);
    void onReceive(const std::string& str, Author author = Author::ASSISTANT);
};