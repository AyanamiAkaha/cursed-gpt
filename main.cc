#include <iostream>
#include <memory>
#include "app.hh"

int main(int argc, char **argv) {
    if(argc != 1) {
        std::cerr << argv[0] <<  "takes no arguments.\n";
        return 1;
    }
    App app;
    auto exit_code = app.run();
    return exit_code;
}