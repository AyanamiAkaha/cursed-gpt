#include "gpt_chat.hh"

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