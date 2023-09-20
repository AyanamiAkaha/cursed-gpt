#include <vector>
#include <httplib.h>
#include <json.hpp>
#include "concurrent_queue.hh"
#include "gpt_chat.hh"

const char* role(Author author) {
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

ApiRequest::ApiRequest(std::unique_ptr<char[]>&& body) :  body(std::move(body)) {}
ApiRequest::ApiRequest(const ApiRequest &req) {
    id = req.id;
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
            {"role", role(message.author)},
            {"content", message.message}
        });
    }
    auto str = json.dump();
    auto len = str.length();
    body = std::make_unique<char[]>(len);
    std::memcpy(body.get(), str.c_str(), len);
}

GptChat::GptChat(std::string name) :
    Chat(name),
    reqQueue(),
    resQueue(),
    client("https://api.openai.com/v1/chat/completions")
{
    client.set_connection_timeout(5);
    client.set_read_timeout(5);
    client.set_write_timeout(5);
}

GptChat::~GptChat() {}

void GptChat::send(const std::string &str, Author author)
{
    Chat::send(str, author);
    std::vector<Message> allMessages(template_messages.size() + messages.size());
    allMessages.insert(allMessages.end(), template_messages.begin(), template_messages.end());
    allMessages.insert(allMessages.end(), messages.begin(), messages.end());
    ApiRequest req = ApiRequest(allMessages);
    reqQueue << req;
}

void GptChat::onReceive(const std::string &str, Author author)
{
    Chat::onReceive(str, author);
    // TODO
}