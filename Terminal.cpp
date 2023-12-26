//
// Created by actre on 12/3/2023.
//


#include "Terminal.h"

#include <ranges>
#include <filesystem>
#include <bitset>


#define HELP_CMD_MAX_LENGTH 12

using std::endl;

namespace FileSystem {

    Terminal::Terminal(std::ostream &os) : os(os) {
        initRouterAndDocs();
    }

    bool Terminal::putCommand(const std::string &cmd) {

        auto decompose = commandTrim(splitString(cmd, '#').front());

        if (decompose.first.empty()) return true;

        try {
            auto target = router.at(decompose.first);
            target(decompose.second);
        } catch (std::out_of_range &) {
            os << "未知命令：" << decompose.first << endl;
        } catch (Error &e) {
            os << "发生异常：" << endl;
            os << e.what() << endl;
        }
        return false;
    }

    bool Terminal::enterCommand(std::istream &is) {
        os << localPrefixBuilder();
        std::string cmd;
        std::getline(is, cmd);

        return putCommand(cmd);
    }

    void Terminal::putScript(const std::string &script) {
        int line = 0;
        os << script << endl;
        for (const auto &statement: splitString(script, '\n')) {
            os << statement << endl;
            line++;
            try {
                putCommand(statement);
            } catch (Error &) {
                os << "运行到第 " << line << " 行发生错误！" << endl;
                break;
            }
        }
    }

    void Terminal::runScript(const std::string &path) {
        std::ifstream f(path);
        int line = 0;
        while (!f.eof()) {
            std::string statement;
            std::getline(f, statement, '\n');
            line++;
            if (!putCommand(statement)) {
                os << "运行到第 " << line << " 行发生错误: " << statement << endl;
                break;
            }
        }
        f.close();
    }

    int Terminal::assertArgSize(const std::list<std::string> &args, const std::vector<int> &allowSizes,
                                const std::string &cmd) {
        if (std::any_of(allowSizes.begin(), allowSizes.end(),
                        [&args](const auto &it) -> bool { return args.size() == it; }))
            return args.size();

        throw Error{"Terminal::assertArgSize", "不支持的参数数量：输入 [help " + cmd + "] 查看指令详细用法"};
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
                "ls {可选：目录名}\n"
                "显示目标目录下的项目"
        };

        router["mkdir"] = [this](const auto &args) { mkdir(args); };
        docs["mkdir"] = {
                "创建新的文件夹",
                "mkdir [文件夹名称]\n"
                "在当前目录下创建新的文件夹"
        };

        router["exit"] = [](const auto &args) { exit(args); };
        docs["exit"] = {
                "离开",
                "exit\n"
                "点开连接并退出程序"
        };

        router["cd"] = [this](const auto &args) { cd(args); };
        docs["cd"] = {
                "进入目录",
                "cd [目录]\n"
                "进入到目标目录"
        };

        router["script"] = [this](const auto &args) { script(args); };
        docs["script"] = {
                "执行脚本",
                "script [external / internal] [文件地址]\n"
                "执行文件系统脚本\n"
                "参数为 external 时将执行外部文件\n"
                "参数为 internal 时将执行内部文件(权限必须为可执行)"
        };

        router["upload"] = [this](const auto &args) { upload(args); };
        docs["upload"] = {
                "从外部上传文件",
                "upload [外部文件地址] {可选: 文件路径 / 文件名}\n"
                "从外部上传文件"
        };

        router["rm"] = [this](const auto &args) { rm(args); };
        docs["rm"] = {
                "删除目标文件",
                "rm [文件地址]\n"
                "不可用于删除文件夹"
        };

        router["struct"] = [this](const auto &args) { printstruct(args); };
        docs["struct"] = {
                "打印磁盘结构",
                "struct\n"
                "输出磁盘结构"
        };

        router["clear"] = [](const auto &args) { clear(args); };
        docs["clear"] = {
                "清空终端",
                "clear\n"
                "清空终端"
        };

        router["rmdir"] = [this](const auto &args) { rmdir(args); };
        docs["rmdir"] = {
                "删除目录",
                "rmdir [目录]\n"
                "删除目录"
        };

        router["edit"] = [this](const auto &args) { edit(args); };
        docs["edit"] = {
                "编辑文件，文件不存在自动创建",
                "edit [文件路径] {临时文件后缀}\n"
                "使用外部编辑器编辑文本文件\n"
                "文件不存在则自动创建\n"
                "使用 Terminal::assignEditor 设置外部编辑器\n"
                "使用 Terminal::assignTempFolder 设置外部临时文件夹"
        };

        router["editdone"] = [this](const auto &args) { editdone(args); };
        docs["editdone"] = {
                "保存文件编辑，释放文件写锁",
                "editdone\n"
                "结束文件编辑，释放文件写锁\n"
                "执行后无论是否失败，均会释放文件写锁"
        };

        router["editcancel"] = [this](const auto &args) { editcancel(args); };
        docs["editcancel"] = {
                "取消文件编辑，释放文件写锁",
                "editcancel\n"
                "取消文件编辑，释放文件写锁"
        };

        router["su"] = [this](const auto &args) { su(args); };
        docs["su"] = {
                "将身份切换到超级用户",
                "su [超级用户密码]\n"
                "将身份切换到超级用户"
        };

        router["us"] = [this](const auto &args) { us(args); };
        docs["us"] = {
                "将身份切换到普通用户",
                "us\n"
                "将身份切换到普通用户"
        };

        router["chmod"] = [this](const auto &args) { chmod(args); };
        docs["chmod"] = {
                "设置文件权限（管理员）",
                "chmod [2位或6位权限代码] [目标文件]\n"
                "设置文件权限，权限代码前半部分为管理员权限，后半部分为用户权限\n"
                "每部分对应的3位分别为 可读、可编辑、可执行"
        };

        router["login"] = [this](const auto &args) { login(args); };
        docs["login"] = {
                "登录用户",
                "login [用户名] [密码]\n"
                "登录用户"
        };

        router["register"] = [this](const auto &args) { reg(args); };
        docs["register"] = {
                "注册用户（管理员）",
                "register [用户名] [密码]\n"
                "创建新的用户\n"
                "管理员命令"
        };

        router["format"] = [this](const auto &args) { format(args); };
        docs["format"] = {
                "格式化硬盘（管理员）",
                "format [管理员密码]\n"
                "保持硬盘大小不变，格式化硬盘。"
        };

        router["cat"] = [this](const auto &args) { cat(args); };
        docs["cat"] = {
                "输出文件内容",
                "cat [文件路径]\n"
                "保持硬盘大小不变，格式化硬盘。"
        };


    }

    std::string Terminal::localPrefixBuilder() {
        if (!controller.good()) {
            return "[ 未链接 ] > ";
        } else {
            return "[ " + controller.getTitle() + " ] " + getUrl() + " > ";
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

    std::list<std::string> Terminal::parseUrl(const std::string &url) {

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

    void Terminal::assertConnection() {
        assert(controller.good(), "Terminal::assertConnection",
               "当前未链接到文件系统，请使用 link 或 create 链接、创建文件系统。");
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

        assertArgSize(args, {1}, "link");

        const std::string &pathHolder = args.front();

        DiskEntity diskEntity{pathHolder};
        controller.setPath(pathHolder);
        os << "链接成功！" << endl;

        resetUrl();
    }

    void Terminal::create(const std::list<std::string> &args) {

        assertArgSize(args, {3}, "create");

        auto iter = args.begin();
        std::string pathHolder = *(iter++);
        std::string sizeStr = *(iter++);
        std::string rootPassword = *iter;

        try {
            u_int64 size = parseSizeString(sizeStr);
            controller.create(size, pathHolder, rootPassword);
            os << "创建成功！" << endl;
        } catch (size_format_error &) {

            throw Error{"Terminal::create", "非法的大小输入: " + sizeStr};
        }

        resetUrl();
    }

    void Terminal::ls(const std::list<std::string> &args) {

        assertConnection();

        std::list<std::string> target;

        auto argSize = assertArgSize(args, {0, 1}, "ls");

        if (argSize == 1) {
            target = parseUrl(args.front());
        } else {
            target = sessionUrl;
        }

        auto targetStr = pathStr(target);

        auto dirs = controller.getDir(target);
        if (dirs.empty()) {
            os << "目录 " + targetStr + " 下为空" << endl;
        } else {
            os << "目录 " + targetStr + " 下共有 " + std::to_string(dirs.size()) + " 个项目" << endl;
            for (const auto &inode: dirs) {
                os << inode.name;
                if (inode.getType() == INode::Folder) {
                    os << '/';
                }
                os << endl;
            }
        }
    }

    void Terminal::mkdir(const std::list<std::string> &args) {

        assertConnection();

        assertArgSize(args, {1}, "mkdir");

        controller.createDir(sessionUrl, args.front());
        os << "目录创建成功" << endl;
    }

    void Terminal::exit(const std::list<std::string> &) {
        throw ExitSignal();
    }

    void Terminal::cd(const std::list<std::string> &args) {

        assertConnection();
        assertArgSize(args, {1}, "cd");

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

    void Terminal::script(const std::list<std::string> &args) {

        assertArgSize(args, {2}, "script");

        const auto &path = args.back();

        if (args.front() == "external") {
            assert(std::filesystem::exists(path), "Terminal::script", "目标脚本不存在！");

            os << "开始执行脚本：" << path << endl;
            runScript(path);
            os << "脚本执行完毕。" << endl;
        } else if (args.front() == "internal") {

            putScript(controller.getScript(parseUrl(path)));

        } else {
            throw Error{"Terminal::script", "首个参数必须为 external 或 internal ，详见 help script"};
        }
    }

    void Terminal::assignEditor(const std::string &editorApplication, const std::string &withArgs) {

        editCmdApp = '\"' + editorApplication + '\"';

        if (!withArgs.empty()) {
            editCmdApp.append(" " + withArgs);
        }

        editCmdApp += " ";

        editExternalFile = [this](const std::string &path) {
            auto cmd = editCmdApp + path;
            os << "执行命令： " << cmd << endl;
            system(cmd.c_str());
        };

        editExternalFileAvailable = true;
    }

    void Terminal::assignTempFolder(const std::string &_folderPath) {

        std::string folderPath = _folderPath.ends_with('/') ?
                                 _folderPath.substr(0, _folderPath.size() - 1) :
                                 _folderPath;

        assert(
                !std::filesystem::is_regular_file(folderPath),
                "Terminal::assignTempFolder",
                "设置的临时文件夹目录为文件"
        );

        if (!std::filesystem::is_directory(folderPath)) {
            assert(
                    std::filesystem::create_directory(folderPath),
                    "Terminal::assignTempFolder",
                    "临时文件夹创建失败"
            );
        }

        tempFolder = _folderPath;
    }

    void Terminal::upload(const std::list<std::string> &args) {

        assertConnection();

        std::string fileName;
        std::list<std::string> targetPath;

        auto argSize = assertArgSize(args, {1, 2}, "upload");

        if (argSize == 1) {
            fileName = splitString(args.front(), '/').back();
            targetPath = sessionUrl;
        } else {
            targetPath = parseUrl(args.back());
            fileName = targetPath.back();
            targetPath.pop_back();
        }

        auto fSize = std::filesystem::file_size(args.front());

        std::ifstream f{args.front(), std::ios::in | std::ios::binary};

        auto data = ByteArray().read(f, fSize, false);

        f.close();

        controller.createFile(targetPath, fileName, data);
    }

    void Terminal::rm(const std::list<std::string> &args) {

        assertConnection();

        assertArgSize(args, {1}, "rm");

        auto targetFileUrl = parseUrl(args.front());

        controller.removeFile(targetFileUrl, false, &os);
    }

    void Terminal::printstruct(const std::list<std::string> &) {

        assertConnection();

        controller.printStructure(os);
    }

    void Terminal::clear(const std::list<std::string> &) {
        clearConsole();
    }

    void Terminal::resetUrl() {
        sessionUrl.clear();
    }

    void Terminal::rmdir(const std::list<std::string> &args) {

        assertConnection();

        assertArgSize(args, {1}, "rmdir");

        auto targetFileUrl = parseUrl(args.front());

        controller.removeDir(targetFileUrl, &os);
    }

    void Terminal::edit(const std::list<std::string> &args) {

        assertConnection();

        assertEditable();

        assertLazy(editSession == nullptr, "Terminal::edit",
                   [this]() { return "当前已存在正在编辑的文件：" + editSession->getFileName(); });

        auto argSize = assertArgSize(args, {1, 2}, "edit");

        auto targetUrl = parseUrl(args.front());

        auto fileName = targetUrl.back();

        editSession = new FSController::EditSession{controller.editFile(targetUrl)};

        tempFileName = tempFolder + randomStr(16);

        if (argSize == 2) {
            tempFileName += ("." + args.back());
        } else {
            auto splitParts = splitString(fileName, '.');
            if (splitParts.size() >= 2) {
                tempFileName += ("." + splitParts.back());
            } else {
                tempFileName += ".txt";
            }
        }

        std::ofstream tempFile{tempFileName, std::ios::out | std::ios::trunc};

        assert(tempFile.is_open(), "Terminal::edit", "临时文件创建失败");

        auto fileData = editSession->getFileData();

        tempFile.write(reinterpret_cast<char *>(fileData.data()), fileData.flatSize());

        tempFile.close();

        editExternalFile(tempFileName);

        os << "已开始编辑文件\n编辑结束后，输入 editdone 以保存编辑，输入 editcancel 以取消编辑。" << endl;
    }

    void Terminal::assertEditable() {
        assert(editExternalFileAvailable, "Terminal::assertEditable", "未声明外部文件编辑器");
        assert(!tempFolder.empty(), "Terminal::assertEditable", "未声明临时文件夹");
    }

    void Terminal::su(const std::list<std::string> &args) {

        assertConnection();

        assertArgSize(args, {1}, "su");

        controller.changeRole(INode::Admin, args.front());
    }

    void Terminal::us(const std::list<std::string> &) {

        assertConnection();

        controller.changeRole(INode::User);
    }

    void Terminal::editdone(const std::list<std::string> &args) {

        assertConnection();

        assert(editSession != nullptr, "Terminal::edit", "当前不存在正在编辑的文件");

        if (!std::filesystem::is_regular_file(tempFileName)) {
            tempFileName = "";
            editSession->cancelEdit();
            delete editSession;
            editSession = nullptr;
            throw Error{"Terminal::editdone", "更新失败：临时文件被销毁，文件写锁已被重置！"};
        }

        auto newFileSize = std::filesystem::file_size(tempFileName);

        char *buf = new char[newFileSize];

        std::ifstream i{tempFileName};

        i.read(buf, newFileSize);

        i.close();

        try {
            editSession->assignEditFinish(ByteArray(reinterpret_cast<std::byte *>(buf), newFileSize));
        } catch (Error &e) {
            std::filesystem::remove(tempFileName);
            tempFileName = "";
            delete editSession;
            editSession = nullptr;
            throw Error{"Terminal::editdone", std::string{"更新失败！文件写锁已被重置！\n 错误原因："} + e.what()};
        }
        std::filesystem::remove(tempFileName);
        tempFileName = "";
        delete editSession;
        editSession = nullptr;
    }

    void Terminal::editcancel(const std::list<std::string> &args) {

        assertConnection();

        assert(editSession != nullptr, "Terminal::edit", "当前不存在正在编辑的文件");

        if (std::filesystem::is_regular_file(tempFileName)) {
            std::filesystem::remove(tempFileName);
        }

        tempFileName = "";
        editSession->cancelEdit();
        delete editSession;
        editSession = nullptr;

    }

    void Terminal::chmod(const std::list<std::string> &args) {
        assertArgSize(args, {2}, "chmod");
        assertConnection();

        assert(args.front().size() == 2 || args.front().size() == 6, "Terminal::chmod", "第一个参数必须为2位或6位");
        unsigned int adminPermissionNum;
        unsigned int userPermissionNum;
        if (args.front().size() == 2) {
            adminPermissionNum = args.front()[0] - '0';
            assert(adminPermissionNum < 0x0f, "Terminal::chmod", "输入的权限不合规");
            userPermissionNum = args.front()[1] - '0';
            assert(userPermissionNum < 0x0f, "Terminal::chmod", "输入的权限不合规");
        } else {
            auto permissionStr = args.front();

            adminPermissionNum = 0;
            for (int i = 0; i < 3; ++i) {
                adminPermissionNum = adminPermissionNum << 1;
                assert(permissionStr[i] == '0' || permissionStr[i] == '1', "Terminal::chmod", "输入的权限不合规");
                adminPermissionNum = adminPermissionNum | (permissionStr[i] == '1');
            }

            userPermissionNum = 0;
            for (int i = 3; i < 6; ++i) {
                userPermissionNum = userPermissionNum << 1;
                assert(permissionStr[i] == '0' || permissionStr[i] == '1', "Terminal::chmod", "输入的权限不合规");
                userPermissionNum = userPermissionNum | (permissionStr[i] == '1');
            }
        }

        unsigned char permissionCode = (adminPermissionNum << 4) | userPermissionNum;
        controller.setFilePermission(parseUrl(args.back()),
                                     INode::PermissionGroup::fromByte(static_cast<std::byte>(permissionCode)));
    }

    void Terminal::format(const std::list<std::string> &args) {
        assertArgSize(args, {1}, "format");

        assertConnection();

        controller.format(args.front());
    }

    void Terminal::login(const std::list<std::string> &args) {
        assertArgSize(args, {2}, "login");
        assertConnection();
        assert(controller.login(args.front(), args.back()), "Terminal::login", "密码错误！");
    }

    void Terminal::reg(const std::list<std::string> &args) {
        assertArgSize(args, {2}, "register");
        assertConnection();
        controller.registerUser(args.front(), args.back());
    }

    void Terminal::cat(const std::list<std::string> &args) {
        assertConnection();
        assertArgSize(args, {1}, "cat");
        os << controller.cat(parseUrl(args.front())) << endl;
    }


}