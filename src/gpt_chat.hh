#pragma once

#include <atomic>
#include <string>

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "concurrent_queue.hh"
#include "chat.hh"

class ApiRequest {
private:
    unsigned int chatId;
    std::unique_ptr<char[]> body;
public:
    ApiRequest(unsigned int chatId, std::unique_ptr<char[]>&& body);
    ApiRequest(const ApiRequest& req);
    ApiRequest(const std::vector<Message>& messages);
    ~ApiRequest() = default;
};

class ApiResponse {
private:
    unsigned int chatId;
    std::unique_ptr<char[]> body;
public:
    ApiResponse(unsigned int chatId, std::unique_ptr<char[]>&& body);
    ApiResponse(const ApiResponse& res);
    ~ApiResponse() = default;
};

class GptChat : public Chat {
private:
    httplib::Client client;
    ConcurrentQueue<ApiRequest> reqQueue;
    ConcurrentQueue<ApiResponse> resQueue;
public:
    GptChat(std::string name = "GptChat");
    ~GptChat();
    static const std::string role(Author author);
    void send(const std::string& str, Author author = Author::USER);
    void onReceive(const std::string& str, Author author = Author::ASSISTANT);
};