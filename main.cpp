#include <iostream>

#include "Terminal.h"

int main() {
    FileSystem::Terminal cs{std::cout};

    while (true) {
        cs.enterCommand(std::cin);
    }

    return 0;
}
// create D:/test.sfs 12MB abc123