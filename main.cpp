#include <iostream>

#include "Terminal.h"

#if 1

int main() {

    FileSystem::Terminal cs{std::cout};

    while (true) {
        try {
            cs.enterCommand(std::cin);
        } catch (FileSystem::ExitSignal &) {
            break;
        }
    }

    return 0;
}

#else

int main() {

    FileSystem::Terminal cs{std::cout};

    cs.runScript("D:/script.sfss");

    return 0;
}

#endif