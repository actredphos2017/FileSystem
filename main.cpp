#include <iostream>

#include "Terminal.h"

int main() {
    FileSystem::Terminal cs{std::cout};

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
