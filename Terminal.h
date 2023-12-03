//
// Created by actre on 12/3/2023.
//

#ifndef FILESYSTEM_TERMINAL_H
#define FILESYSTEM_TERMINAL_H

#include <iostream>
#include <unordered_map>
#include <functional>

#include "Utils.h"
#include "FileSystemConnector.h"

namespace FileSystem {

    typedef std::unordered_map<std::string, std::function<std::string(const std::list<std::string> &)>> Router;

    typedef std::unordered_map<std::string, std::pair<std::string, std::string>> DocMap;

    class Terminal {

    public:

        explicit Terminal(std::ostream &os);

        void enterCommand(std::istream &is);

    private:

        std::ostream &os;

        FileSystemConnector *connector{nullptr};
        std::list<std::string> sessionUrl{};

        std::string getUrl();

        Router router{};
        DocMap docs{};

        void initRouterAndDocs();

        std::string localPrefixBuilder();

        std::string help(const std::list<std::string> &args);

        std::string link(const std::list<std::string> &args);

        std::string create(const std::list<std::string> &args);
    };
}


#endif //FILESYSTEM_TERMINAL_H
