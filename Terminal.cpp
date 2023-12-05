//
// Created by actre on 12/3/2023.
//


#include "Terminal.h"

using std::endl;

namespace FileSystem {
    Terminal::Terminal(std::ostream &os) : os(os) {
        initRouterAndDocs();
    }

    void Terminal::enterCommand(std::istream &is) {
        os << localPrefixBuilder();
        std::string cmd;
        std::getline(is, cmd);

        auto decompose = commandTrim(cmd);

        if (decompose.first.empty()) return;

        try {
            auto target = router.at(decompose.first);
            target(decompose.second);
        } catch (std::out_of_range &) {
            os << "未知命令：" << decompose.first << endl;
        } catch (Error &e) {
            os << "发生异常：" << endl;
            os << e.what() << endl;
        }
    }

    void Terminal::initRouterAndDocs() {
        router["help"] = [this](const auto &args) { help(args); };

        router["link"] = [this](const auto &args) { link(args); };
        docs["link"] = {
                "链接到目标文件系统。",
                "link [文件系统路径]"
                "使用这个命令让终端连接到一个文件系统。"
                "你可以像这样使用该命令： \"link D:/mySavedFileSystem.sfs\""
                "如果没有链接文件系统，系统无法工作。"
                "如果没有存在的文件系统，可通过 \"create\" 命令来创建一个。"
                "输入 \"help create\" 查看更多信息。"
        };

        router["create"] = [this](const auto &args) { create(args); };
        docs["create"] = {
                "创建一个新的文件系统",
                "create [文件系统创建路径] [文件系统大小] [管理员密码]\n"
                "使用这个命令来创建一个新的文件系统\n"
                "你可以像这样使用该命令 \"create D:/myFileSystem.sfs 512MB abc123\"\n"
                "文件系统至少需要 8KB 大小才能工作\n"
                "创建完成后终端将自动连接该文件系统"
        };

        router["ls"] = [this](const auto &args) { ls(args); };
        docs["ls"] = {
                "显示当前目录下的文件",
                "ls\n"
                "显示当前目录下的文件"
        };

        router["dbpg"] = [this](const auto &args) { dbpg(args); };
    }

    void Terminal::help(const std::list<std::string> &args) {
        if (args.empty()) {
            for (const auto &doc: docs) {
                os << "  - " << doc.first << "\t:" << doc.second.first << endl;
            }
            os << endl;
            os << "输入 \"help [目标命令]\" 来获取目标命令的详细用法。" << endl;
        } else {
            try {
                auto targetDoc = docs.at(args.front());
                os << "指令 " << args.front() << " 的详细信息：" << endl;
                os << targetDoc.second << endl;
            } catch (std::out_of_range &) {
                os << "无法找到有关指令 " << args.front() << " 的文档。" << endl;
            }
        }
    }

    void Terminal::dbpg(const std::list<std::string> &args) {
        os << controller._diskEntity->_fileLinker.path << endl;
    }

    void Terminal::link(const std::list<std::string> &args) {
        if (args.size() != 1) {
            os << "指令需要且仅需要一个参数才能执行！" << endl;
            os << "使用 help link 查看更多信息" << endl;
            return;
        }

        std::string pathHolder = args.front();

        DiskEntity diskEntity{pathHolder};
        controller.setPath(pathHolder);
        os << "链接成功！" << endl;
    }

    void Terminal::create(const std::list<std::string> &args) {
        if (args.size() != 3) {
            os << "指令需要三个参数才能执行\n" << endl;
            os << "使用 help create 查看更多信息" << endl;
        }
        auto iter = args.begin();
        std::string pathHolder = *(iter++);
        std::string sizeStr = *(iter++);
        std::string rootPassword = *iter;

        try {
            u_int64 size = parseSizeString(sizeStr);
            controller.create(size, pathHolder, rootPassword);
            os << "创建成功！" << endl;
        } catch (size_format_error &) {
            os << "非法的大小输入: " << sizeStr << endl;
        }
    }

    std::string Terminal::localPrefixBuilder() {
        std::stringstream ss;
        if (!controller.good()) {
            ss << "UNLINK > ";
        } else {
            ss << getUrl() << " > ";
        }
        return ss.str();
    }

    std::string Terminal::getUrl() {
        std::stringstream ss;
        ss << "/";
        for (const auto &part: sessionUrl) {
            ss << part << "/";
        }
        return ss.str();
    }

    void Terminal::ls(const std::list<std::string> &args) {
        assertConnection();
        auto dirs = controller.getDir(getUrl());
        if (dirs.empty()) {
            os << "当前目录为空" << endl;
        } else {
            for (const auto &dir: dirs) {
                const auto &inode = dir.second;
                os << inode.getName();
                if (inode.getType() == INode::Path) {
                    os << '/';
                }
                os << endl;
            }
        }
    }

    void Terminal::assertConnection() {
        assert(controller.good(), "Terminal::ls", "当前未链接到文件系统，请使用 link 或 create 链接、创建文件系统。");
    }
}