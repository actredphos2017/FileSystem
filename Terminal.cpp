//
// Created by actre on 12/3/2023.
//


#include "Terminal.h"


namespace FileSystem {
    Terminal::Terminal(std::ostream &os) : os(os) {
        initRouterAndDocs();
    }

    void Terminal::enterCommand(std::istream &is) {
        os << localPrefixBuilder();
        std::string cmd;
        std::getline(is, cmd);

        auto decompose = commandTrim(cmd);

        std::string response{};

        try {
            auto target = router.at(decompose.first);
            response = target(decompose.second);
        } catch (std::out_of_range &) {
            response = "Unknown command: " + decompose.first;
        }

        os << response;
    }

    void Terminal::initRouterAndDocs() {
        router["help"] = std::bind(&Terminal::help, this, std::placeholders::_1);

        router["link"] = std::bind(&Terminal::link, this, std::placeholders::_1);
        docs["link"] = {
                "Link Terminal to the targeted virtual file system file.",
                "link [File System Path]"
                "Use this command to let connector link to a saved file system file."
                "You can use this command like \"link D:/mySavedFileSystem.sfs\" in Windows."
                "System will not work without an existed file system."
                "If there is not an existed file, just enter \"create\" command to create one."
                "Enter \"help create\" to show more detail."
        };

        router["create"] = std::bind(&Terminal::create, this, std::placeholders::_1);
        docs["create"] = {
                "Create a new file system.",
                "create [File System Path] [Size] [Super User Password]"
                "Use this command to create a new File System."
                "You can use this command like \"create D:/myFileSystem.sfs 512MB abc123\" in Windows."
                "File System need more than 8KB to work."
                "Terminal will automatically link file."
        };
    }

    std::string Terminal::help(const std::list<std::string> &args) {

        std::stringstream ss;

        if (args.empty()) {
            ss << "===== HELP =====" << std::endl;
            for (const auto &doc: docs) {
                ss << "  - " << doc.first << "\t:" << doc.second.first << std::endl;
            }
            ss << std::endl;
            ss << "Enter help [Target Command] to show more details of target command." << std::endl;
        } else {
            try {
                auto targetDoc = docs.at(args.front());
                ss << "Help of command: " << args.front() << std::endl;
                ss << targetDoc.second << std::endl;
            } catch (std::out_of_range &) {
                ss << "Cannot find doc to command: " << args.front() << std::endl;
            }
        }

        return ss.str();
    }

    std::string Terminal::link(const std::list<std::string> &args) {
        if (args.size() != 1) {
            return "Commend require and take only one argument to run!"
                   "Show more detail: help link";
        }

        try {
            DiskEntity diskEntity{args.front()};
            connector = new FileSystemConnector{diskEntity};
            return "Link Successfully!";
        } catch (std::exception &e) {
            return std::string{"Exception Happened! What:\n"} + e.what();
        }
    }

    std::string Terminal::create(const std::list<std::string> &args) {
        if (args.size() != 3) {
            return "Commend require and take only 3 argument to run!"
                   "Show more detail: help create";
        }
        auto iter = args.begin();
        std::string path = *(iter++);
        std::string sizeStr = *(iter++);
        std::string rootPassword = *iter;

        try {
            u_int64 size = parseSizeString(sizeStr);

            DiskEntity diskEntity{size, path, rootPassword};
            connector = new FileSystemConnector{diskEntity};
            return "Create Successfully!";
        } catch (size_format_error &) {
            return "Illegal size input: " + sizeStr;
        } catch (std::exception &e) {
            return std::string{"Exception Happened! What:\n"} + e.what();
        }
    }

    std::string Terminal::localPrefixBuilder() {
        std::stringstream ss{"\n"};
        if (connector == nullptr) {
            ss << "UNLINK > ";
        } else {
            ss << getUrl() << " > ";
        }
        return  ss.str();
    }

    std::string Terminal::getUrl() {
        std::stringstream ss;
        ss << "/";
        for (const auto &part: sessionUrl) {
            ss << part << "/";
        }
        return ss.str();
    }
}