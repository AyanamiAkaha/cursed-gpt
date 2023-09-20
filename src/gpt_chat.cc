#include <httplib.h>
#include <json.hpp>
#include "gpt_chat.hh"

ApiRequest::ApiRequest(const Message &message) : message(message) {}
ApiRequest::ApiRequest(const ApiRequest &req) : message(req.message) {}
ApiRequest::~ApiRequest() {}
std::string ApiRequest::getJson() const
{
    nlohmann::json json;
    json["message"] = message.message;
    json["author"] = (int)message.author;
    json["timestamp"] = message.timestamp;
    json["id"] = message.id;
    return json.dump();
}

GptChat::GptChat(std::string name) : Chat(name)
{
}

GptChat::~GptChat()
{
}

void GptChat::send(const std::string &str, Author author)
{
    Chat::send(str, author);
    // TODO
}

void GptChat::onReceive(const std::string &str, Author author)
{
    Chat::onReceive(str, author);
    // TODO
}