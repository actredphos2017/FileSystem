#include "Terminal.h"

using namespace FileSystem;

// script external D:/script.sfss

int main() {

    Terminal terminal{std::cout};

    terminal.assignEditor("C:/Program Files/Sublime Text/sublime_text.exe");
    terminal.assignTempFolder("Temp/");

    while (true) {
        try {
            terminal.enterCommand(std::cin);
        } catch (ExitSignal &) {
            break;
        }
    }

    return 0;
}