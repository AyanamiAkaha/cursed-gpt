#include "chat.hh"

class GptChat : public Chat {
private:
public:
    GptChat(std::string name = "GptChat");
    ~GptChat();
    void send(const std::string& str, Author author = Author::USER);
    void onReceive(const std::string& str, Author author = Author::ASSISTANT);
}