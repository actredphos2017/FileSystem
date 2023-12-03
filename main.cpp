#include <iostream>

#include "CommandService.h"

int main() {
    FileSystem::CommandService cs{std::cout};

    while (true) {
        try {
            cs.enterCommand(std::cin);
        } catch (std::exception& e) {
            e.what();
            break;
        }
    }

    return 0;
}
