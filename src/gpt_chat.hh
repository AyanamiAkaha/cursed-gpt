#pragma once

#include <atomic>
#include <thread>
#include <string>

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "concurrent_queue.hh"
#include "chat.hh"

class ApiRequest {
private:
    unsigned int chatId;
    std::unique_ptr<char[]> body;
public:
    ApiRequest() = delete;
    ApiRequest(unsigned int chatId, std::unique_ptr<char[]>&& body);
    ApiRequest(const ApiRequest& req);
    ApiRequest(const std::vector<Message>& messages);
    ~ApiRequest() = default;
    std::string getBody() const { return std::string(body.get()); }
    std::unique_ptr<char[]> getBodyC();
    unsigned int getChatId() const { return chatId; }
};

class ApiResponse {
private:
    unsigned int chatId;
    std::unique_ptr<char[]> body;
public:
    ApiResponse(unsigned int chatId, std::unique_ptr<char[]>&& body);
    ApiResponse(const ApiResponse& res);
    ~ApiResponse() = default;
    std::string getBody() const { return std::string(body.get()); }
};

class GptChat : public Chat {
private:
    httplib::Client client;
    ConcurrentQueue<ApiRequest> reqQueue;
    ConcurrentQueue<ApiResponse> resQueue;
    std::thread networkThread;
    void networkLoop();
public:
    GptChat(std::string name = "GptChat");
    ~GptChat();
    static std::string role(const Author author);
    static Author author(const std::string role);
    void send(const std::string& str, Author author = Author::USER);
    void checkReceived();
};