//
// Created by actre on 12/3/2023.
//


#include "CommandService.h"


namespace FileSystem {
    CommandService::CommandService(std::ostream &os) : os(os) {
        initRouterAndDocs();
    }

    void CommandService::enterCommand(std::istream &is) {
        os << localPrefixBuilder();
        std::string cmd;
        std::getline(is, cmd);

        auto decompose = commandTrim(cmd);

        try {
            auto target = router.at(decompose.first);


        } catch (std::out_of_range) {

        }

    }

    void CommandService::initRouterAndDocs() {
        router["help"] = [this](auto && PH1) { return help(std::forward<decltype(PH1)>(PH1)); };
    }

    std::string CommandService::help(const std::list<std::string> &args) {

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
            } catch (std::out_of_range) {
                ss << "Cannot find doc to command: " << args.front() << std::endl;
            }
        }

        return ss.str();
    }
}