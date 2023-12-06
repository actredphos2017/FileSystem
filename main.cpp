#include <iostream>

#include "Terminal.h"



int main() {
    FileSystem::Terminal cs{std::cout};

    cs.putCommand("link D:/a.sfs");
    system("pause");
    cs.putCommand("mkdir b");
    system("pause");

    return 0;
}
// create D:/test.sfs 12MB abc123