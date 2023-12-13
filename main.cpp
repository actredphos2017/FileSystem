#include "Terminal.h"

using namespace FileSystem;

// script external D:/script.sfss

int main() {

    Terminal cs{std::cout};

    cs.assignEditor("\"C:/Program Files/Sublime Text/sublime_text.exe\"");
    cs.assignTempFolder("Temp/");

    while (true) {
        try {
            cs.enterCommand(std::cin);
        } catch (ExitSignal &) {
            break;
        }
    }

    return 0;
}