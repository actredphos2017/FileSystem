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

        bool putCommand(const std::string &cmd);

        bool enterCommand(std::istream &is);

        void putScript(const std::string &script);

        void runScript(const std::string &path);

        void assignEditor(const std::string &editorApplication, const std::string &withArgs = "");

        void assignTempFolder(const std::string &_folderPath);


        void help(const std::list<std::string> &args);

        void link(const std::list<std::string> &args);

        void create(const std::list<std::string> &args);

        void ls(const std::list<std::string> &args);

        void mkdir(const std::list<std::string> &args);

        void cd(const std::list<std::string> &args);

        void script(const std::list<std::string> &args);

        void upload(const std::list<std::string> &args);

        void download(const std::list<std::string> &args);

        void edit(const std::list<std::string> &args);

        void editdone(const std::list<std::string> &args);

        void editcancel(const std::list<std::string> &args);

        void rm(const std::list<std::string> &args);

        void printstruct(const std::list<std::string> &args);

        void rmdir(const std::list<std::string> &args);

        static void clear(const std::list<std::string> &args);

        void su(const std::list<std::string> &args);

        void us(const std::list<std::string> &args);


        static void exit(const std::list<std::string> &args);

    private:

        std::ostream &os;
        std::list<std::string> sessionUrl{};
        std::function<void(const std::string &)> editExternalFile{};
        bool editExternalFileAvailable{false};

        FSController::EditSession *editSession{nullptr};

        std::string tempFolder{};
        std::string editCmdApp{};
        std::string tempFileName{};

        Router router{};
        DocMap docs{};
        FSController controller{};

        std::string getUrl();

        std::string localPrefixBuilder();

        static int
        assertArgSize(const std::list<std::string> &args, const std::vector<int> &allowSizes, const std::string &cmd);

        void resetUrl();

        std::list<std::string> parseUrl(const std::string &url);

        void initRouterAndDocs();

        void assertConnection();

        void assertEditable();
    };

    class ExitSignal : std::exception {
    };

}


#endif //FILESYSTEM_TERMINAL_H
