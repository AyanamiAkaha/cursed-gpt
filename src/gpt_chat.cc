#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <vector>
#include <httplib.h>
#include <json.hpp>
#include "blocking_queue.hh"
#include "gpt_chat.hh"

// API Request ----------
ApiRequest::ApiRequest(unsigned int chatId, std::string body) : chatId(chatId), body(body) {}
ApiRequest::ApiRequest(const ApiRequest &req) : chatId(req.chatId), body(req.body) {}
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
    body = json.dump();
}

// API Response ----------
ApiResponse::ApiResponse(unsigned int chatId, std::string body, ContentType contentType)
    : chatId(chatId), body(body), contentType(contentType) {}
ApiResponse::ApiResponse(const ApiResponse &res)
    : chatId(res.chatId), body(res.body), contentType(res.contentType) {}

// GptChat ----------
GptChat::GptChat(std::string name) :
    Chat(name),
    client("http://api.openai.com"),
    reqQueue(),
    resQueue(),
    networkThread(&GptChat::networkLoop, this)
{
    auto auth = std::getenv("GPT_API_KEY");
    client.set_connection_timeout(5);
    client.set_read_timeout(5);
    client.set_write_timeout(5);
    client.set_follow_location(true);
    client.set_bearer_token_auth(auth);
    client.set_keep_alive(true);
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
    if (res.getContentType() == ContentType::TEXT) {
        addString(res.getBody());
        return;
    }
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
        auto res = client.Post("/v1/chat/completions", req.getBody(), "application/json");
        if(res && res->status == 200) {
            resQueue << ApiResponse(req.getChatId(), res->body, ContentType::JSON);
        } else {
            auto err = httplib::to_string(res.error());
            resQueue << ApiResponse(req.getChatId(), err, ContentType::TEXT);
        }
    }
}