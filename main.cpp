#include <iostream>

#include "Terminal.h"


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
// create D:/test.sfs 12MB abc123