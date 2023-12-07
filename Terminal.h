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

    typedef std::unordered_map<std::string, std::function<bool(const std::list<std::string> &)>> Router;

    typedef std::unordered_map<std::string, std::pair<std::string, std::string>> DocMap;

    class Terminal {

    public:

        explicit Terminal(std::ostream &os);

        bool putCommand(const std::string &cmd);

        bool enterCommand(std::istream &is);

        void putScript(const std::string &script);

        void runScript(const std::string &path);


        bool help(const std::list<std::string> &args);

        bool link(const std::list<std::string> &args);

        bool create(const std::list<std::string> &args);

        bool ls(const std::list<std::string> &args);

        bool mkdir(const std::list<std::string> &args);

        bool cd(const std::list<std::string> &args);

        bool script(const std::list<std::string> &args);


        static bool exit(const std::list<std::string> &args);

    private:

        std::ostream &os;
        std::list<std::string> sessionUrl{};

        Router router{};
        DocMap docs{};
        FSController controller{};

        std::string getUrl();

        std::string localPrefixBuilder();

        std::list<std::string> parseUrl(const std::string& url);

        void initRouterAndDocs();

        void assertConnection();
    };

    class ExitSignal : std::exception {
    };

}


#endif //FILESYSTEM_TERMINAL_H
