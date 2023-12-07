//
// Created by actre on 12/3/2023.
//

#ifndef FILESYSTEM_TERMINAL_H
#define FILESYSTEM_TERMINAL_H

#include <iostream>
#include <unordered_map>
#include <functional>

#include "Utils.h"
#include "FSController.h"

namespace FileSystem {

    typedef std::unordered_map<std::string, std::function<void(const std::list<std::string> &)>> Router;

    typedef std::unordered_map<std::string, std::pair<std::string, std::string>> DocMap;

    class Terminal {

    public:

        explicit Terminal(std::ostream &os);

        void putCommand(const std::string &cmd);

        void enterCommand(std::istream &is);


        void help(const std::list<std::string> &args);

        void link(const std::list<std::string> &args);

        void create(const std::list<std::string> &args);

        void ls(const std::list<std::string> &args);

        void mkdir(const std::list<std::string> &args);

        void cd(const std::list<std::string> &args);

        void debug(const std::list<std::string> &args);

        void exit(const std::list<std::string> &args);


    private:

        std::ostream &os;
        std::list<std::string> sessionUrl{};

        Router router{};
        DocMap docs{};
        FSController controller{};

        std::string getUrl();

        std::string localPrefixBuilder();

        std::list<std::string> parseUrl(std::string url);

        void initRouterAndDocs();

        void assertConnection();
    };

    class ExitSignal : std::exception {
    };

}


#endif //FILESYSTEM_TERMINAL_H
