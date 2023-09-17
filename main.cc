#include <iostream>
#include <memory>
#include "app.hh"

int main(int argc, char **argv) {
    if(argc != 1) {
        std::cerr << argv[0] <<  "takes no arguments.\n";
        return 1;
    }
    auto app = std::unique_ptr<App>();
    auto exit_code = app->run();
    return exit_code;
}