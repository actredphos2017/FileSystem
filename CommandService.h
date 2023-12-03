//
// Created by actre on 12/3/2023.
//

#ifndef FILESYSTEM_COMMANDSERVICE_H
#define FILESYSTEM_COMMANDSERVICE_H

#include <iostream>
#include <unordered_map>
#include <functional>

#include "Utils.h"

namespace FileSystem {

    typedef std::unordered_map<std::string, std::function<std::string(const std::list<std::string> &)>> Router;

    typedef std::unordered_map<std::string, std::pair<std::string, std::string>> DocMap;

    class CommandService {

    public:

        explicit CommandService(std::ostream &os);

        void enterCommand(std::istream &is);

    private:

        std::ostream &os;

        Router router{};
        DocMap docs{};

        void initRouterAndDocs();

        std::string localPrefixBuilder();

        std::string help(const std::list<std::string> &args);

        std::string link(const std::list<std::string> &args);

        std::string create(const std::list<std::string> &args);


    };
}


#endif //FILESYSTEM_COMMANDSERVICE_H
