//
// Created by actre on 12/3/2023.
//


#include "Terminal.h"

#include <ranges>

#define HELP_CMD_MAX_LENGTH 12

using std::endl;

namespace FileSystem {
    Terminal::Terminal(std::ostream &os) : os(os) {
        initRouterAndDocs();
    }

    void Terminal::putCommand(const std::string &cmd) {
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

    void Terminal::enterCommand(std::istream &is) {
        os << localPrefixBuilder();
        std::string cmd;
        std::getline(is, cmd);

        putCommand(cmd);
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
                "显示目录下的项目",
                "ls [可选：目录名]\n"
                "显示目标目录下的项目"
        };

        router["mkdir"] = [this](const auto &args) { mkdir(args); };
        docs["mkdir"] = {
                "创建新的文件夹",
                "mkdir [文件夹名称]\n"
                "在当前目录下创建新的文件夹"
        };

        router["exit"] = [this](const auto &args) { exit(args); };
        docs["exit"] = {
                "离开",
                "exit\n"
                "点开连接并退出程序"
        };

        router["cd"] = [this](const auto &args) { cd(args); };
        docs["cd"] = {
                "进入目录",
                "cd [目录]\n"
                "点开连接并退出程序"
        };

        router["debug"] = [this](const auto &args) { debug(args); };
    }

    void Terminal::help(const std::list<std::string> &args) {
        if (args.empty()) {
            os << "可用命令：" << endl;
            for (const auto &doc: docs) {
                os << "  - " << filledStr(doc.first, HELP_CMD_MAX_LENGTH) << doc.second.first << endl;
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

    void Terminal::link(const std::list<std::string> &args) {
        if (args.size() != 1) {
            os << "指令需要且仅需要一个参数才能执行！" << endl;
            os << "使用 help link 查看更多信息" << endl;
            return;
        }

        const std::string &pathHolder = args.front();

        DiskEntity diskEntity{pathHolder};
        controller.setPath(pathHolder);
        os << "链接成功！" << endl;
    }

    void Terminal::create(const std::list<std::string> &args) {
        if (args.size() != 3) {
            os << "指令需要三个参数才能执行\n" << endl;
            os << "使用 help create 查看更多信息" << endl;
            return;
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
        if (!controller.good()) {
            return "[UNLINK] > ";
        } else {
            return std::format("[{}] {} > ", controller.getDiskTitle(), getUrl());
        }
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

        std::list<std::string> target;

        if (args.empty()) {
            target = sessionUrl;
        } else if (args.size() == 1) {
            target = parseUrl(args.front());
        } else {
            os << "指令不需要或仅需要一个参数才能执行！" << endl;
            os << "使用 help ls 查看更多信息" << endl;
            return;
        }

        auto targetStr = pathStr(target);

        auto dirs = controller.getDir(target);
        if (dirs.empty()) {
            os << std::format("目录 {} 下为空", targetStr) << endl;
        } else {
            os << std::format("目录 {} 下共有 {} 个项目", targetStr, dirs.size()) << endl;
            for (const auto &inode: dirs) {
                os << inode.name;
                if (inode.getType() == INode::Folder) {
                    os << '/';
                }
                os << endl;
            }
        }
    }

    void Terminal::assertConnection() {
        assert(controller.good(), "Terminal::ls", "当前未链接到文件系统，请使用 link 或 create 链接、创建文件系统。");
    }

    void Terminal::mkdir(const std::list<std::string> &args) {
        if (args.size() != 1) {
            os << "指令需要且仅需要一个参数才能执行！" << endl;
            os << "使用 help mkdir 查看更多信息" << endl;
            return;
        }
        assertConnection();

        controller.createDir(sessionUrl, args.front());
        os << "目录创建成功" << endl;
    }

    void Terminal::debug(const std::list<std::string> &args) {
        auto index = atoi(args.front().c_str());
    }

    void Terminal::exit(const std::list<std::string> &args) {
        throw ExitSignal();
    }

    void Terminal::cd(const std::list<std::string> &args) {
        if (args.size() != 1) {
            os << "指令需要且仅需要一个参数才能执行！" << endl;
            os << "使用 help cd 查看更多信息" << endl;
            return;
        }
        assertConnection();

        auto targetPath = parseUrl(args.front());

        if (targetPath.empty()) {
            sessionUrl.clear();
        } else {
            auto inode = controller.getINodeByPath(targetPath);
            assert(inode.getType() == INode::Folder, "Terminal::cd", "目标项目不是文件夹");
            sessionUrl = fixPath(targetPath);
        }
        os << "已到达路径：" << getUrl() << endl;
    }

    std::list<std::string> Terminal::parseUrl(std::string url) {

        auto parse = splitString(url, '/');

        if (parse.front().empty()) {
            // 从 root 开始算
            parse.pop_front();
            return fixPath(parse);
        } else {
            // 从当前开始算
            for (auto &iter: std::ranges::reverse_view(sessionUrl)) {
                parse.push_front(iter);
            }
            return fixPath(parse);
        }
    }


}