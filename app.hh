#ifndef APP_HH
#define APP_HH

struct CommandResult {
    bool _exit;
    int exit_code;
};

class App
{
private:
    CommandResult parseCommand(std::string command);
    std::string prompt() const;
public:
    App();
    ~App();
    int run();
};
#endif