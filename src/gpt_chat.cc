#include <vector>
#include <httplib.h>
#include <json.hpp>
#include "concurrent_queue.hh"
#include "gpt_chat.hh"

// API Request ----------
ApiRequest::ApiRequest(unsigned int chatId, std::unique_ptr<char[]>&& body) :  chatId(chatId), body(std::move(body)) {}
ApiRequest::ApiRequest(const ApiRequest &req) {
    auto len = std::strlen(req.body.get());
    body = std::make_unique<char[]>(len);
    std::memcpy(body.get(), req.body.get(), len);
}
ApiRequest::ApiRequest(const std::vector<Message>& messages) {
    nlohmann::json json;
    json["model"] = "gpt-3.5-turbo";
    json["messages"] = nlohmann::json::array();
    for(auto message : messages) {
        if(message.author == Author::NONE) continue;
        json["messages"].push_back({
            {"role", GptChat::role(message.author)},
            {"content", message.message}
        });
    }
    auto str = json.dump();
    auto len = str.length();
    body = std::make_unique<char[]>(len);
    std::memcpy(body.get(), str.c_str(), len);
}
std::unique_ptr<char[]> ApiRequest::getBodyC() {
    return std::move(body);
}

// API Response ----------
ApiResponse::ApiResponse(unsigned int chatId, std::unique_ptr<char[]>&& body) :  chatId(chatId), body(std::move(body)) {}
ApiResponse::ApiResponse(const ApiResponse &res) {
    auto len = std::strlen(res.body.get());
    body = std::make_unique<char[]>(len);
    std::memcpy(body.get(), res.body.get(), len);
}

// GptChat ----------
GptChat::GptChat(std::string name) :
    Chat(name),
    client("https://api.openai.com/v1/chat/completions"),
    reqQueue(),
    resQueue(),
    networkThread(&GptChat::networkLoop, this)
{
    client.set_connection_timeout(5);
    client.set_read_timeout(5);
    client.set_write_timeout(5);
    auto auth = std::getenv("GPT_API_KEY");
    auto headers = httplib::Headers({
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer " + std::string(auth)}
    });
    client.set_default_headers(headers);
}

GptChat::~GptChat() {}

std::string GptChat::role(const Author author) {
    switch(author) {
        case Author::USER:
            return "user";
        case Author::ASSISTANT:
            return "assistant";
        case Author::SYSTEM:
            return "system";
        default:
            throw std::runtime_error("Invalid author");
    }
}

Author GptChat::author(const std::string role) {
    if(role == "user") {
        return Author::USER;
    } else if(role == "assistant") {
        return Author::ASSISTANT;
    } else if(role == "system") {
        return Author::SYSTEM;
    } else {
        throw std::runtime_error("Invalid role");
    }
}

void GptChat::send(const std::string &str, Author author)
{
    Chat::send(str, author);
    std::vector<Message> allMessages(template_messages.size() + messages.size());
    allMessages.insert(allMessages.end(), template_messages.begin(), template_messages.end());
    allMessages.insert(allMessages.end(), messages.begin(), messages.end());
    ApiRequest req = ApiRequest(allMessages);
    reqQueue << req;
}

void GptChat::checkReceived() {
    Chat::checkReceived();
    if (resQueue.empty()) return;
    ApiResponse res = resQueue.pop();
    auto json = nlohmann::json::parse(res.getBody());
    time_t timestamp = json["created"].get<time_t>();
    auto choices = json["choices"];
    for(auto choice : choices) {
        auto role = choice["message"]["role"].get<std::string>();
        auto content = choice["message"]["content"].get<std::string>();
        auto author = GptChat::author(role);
        addMsg({content, author});
    }
}

void GptChat::networkLoop() {
    while(true) {
        ApiRequest req = reqQueue.pop();
        auto res = client.Post("/complete", req.getBody(), "application/json");
        if(res && res->status == 200) {
            resQueue << ApiResponse(req.getChatId(), req.getBodyC());
        } else {
            std::string err = res ? "Error " + std::to_string(res->status) + ": " +res->reason : "Unknown error";
            std::unique_ptr<char[]> errC(new char[err.length() + 1]);
            errC[err.length()] = '\0';
            std::memcpy(errC.get(), err.c_str(), err.length() + 1);
            resQueue << ApiResponse(req.getChatId(), std::move(errC));
        }
    }
}