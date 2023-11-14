#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <atomic>
#include <thread>
#include <string>
#include <unordered_map>
#include <httplib.h>

#include "blocking_queue.hh"
#include "chat.hh"

using ConfigValue = std::variant<std::string, double>;

class ApiRequest {
private:
    unsigned int chatId;
    std::string body;
public:
    ApiRequest() = delete;
    ApiRequest(unsigned int chatId, std::string body);
    ApiRequest(const ApiRequest& req);
    ApiRequest(const std::string model, double temperature, const std::vector<Message>& messages);
    ~ApiRequest() = default;
    std::string getBody() const { return body; }
    unsigned int getChatId() const { return chatId; }
};

enum class ContentType {
    JSON,
    TEXT
};

class ApiResponse {
private:
    unsigned int chatId;
    std::string body;
    ContentType contentType;
public:
    ApiResponse(unsigned int chatId, std::string body, ContentType contentType);
    ApiResponse(const ApiResponse& res);
    ~ApiResponse() = default;
    std::string getBody() const { return body; }
    ContentType getContentType() const { return contentType; }
};

class GptChat : public Chat {
private:
    static std::unordered_map<std::string, ConfigValue> defaultConfig;
    httplib::Client client;
    BlockingQueue<ApiRequest> reqQueue;
    BlockingQueue<ApiResponse> resQueue;
    std::thread networkThread;
    std::string model() const;
    double temperature() const;
    void networkLoop();
protected:
    bool isValidConfigKVP([[maybe_unused]] const std::string& key, [[maybe_unused]] const ConfigValue& value) const override;
public:
    GptChat(std::string name = "GptChat");
    ~GptChat();
    static std::string role(const Author author);
    static Author author(const std::string role);
    void send(const std::string& str, Author author = Author::USER);
    void checkReceived();
};