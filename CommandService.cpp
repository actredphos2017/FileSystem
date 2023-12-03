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
        router["help"] = std::bind(&CommandService::help, this, std::placeholders::_1);
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
                
            } catch (std::out_of_range) {
                ss << "Cannot find doc to command: " << args.front() << std::endl;
            }
        }

        return ss.str();
    }
}