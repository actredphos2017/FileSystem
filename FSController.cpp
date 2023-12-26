//
// Created by actre on 12/2/2023.
//

#include "FSController.h"
#include "UserTable.h"

#include <ranges>
#include <utility>

namespace FileSystem {

    FSController::EditSession::EditSession(
            ByteArray fileData,
            INode oldINode,
            std::list<std::string> oldPath,
            std::function<bool(const ByteArray &, INode oldINode, std::list<std::string> oldPath)> onFinish,
            std::function<void(std::list<std::string> oldPath)> onCancel) :
            _fileData{std::move(fileData)},
            _onFinish{std::move(onFinish)},
            _oldINode{std::move(oldINode)},
            _onCancel{std::move(onCancel)},
            _oldPath{std::move(oldPath)} {}

    ByteArray FSController::EditSession::getFileData() {
        return _fileData;
    }

    bool FSController::EditSession::assignEditFinish(const ByteArray &newData) {
        return _onFinish(newData, _oldINode, _oldPath);
    }

    std::string FSController::EditSession::getFileName() {
        return pathStr(_oldPath, false);
    }

    void FSController::EditSession::cancelEdit() {
        _onCancel(_oldPath);
    }

    bool FSController::good() const {
        return _diskEntity != nullptr;
    }

    void FSController::create(u_int64 size, std::string path, const std::string &root_password) {
        _diskEntity = new DiskEntity{size, std::move(path), root_password};
        changeRole(INode::Admin, root_password);
        createFile(getUserMapPath(), {}, INode::AdminOnlyPermission);
    }

    void FSController::setPath(std::string path) {
        _diskEntity = new DiskEntity{std::move(path)};
    }

    u_int64 FSController::getFilePos(const std::list<std::string> &_filePath) const {

        auto fixedPath = fixPath(_filePath);

        assert(!fixedPath.empty(), "FSController::getFilePos", "路径非法");

        auto last = fixedPath.back();

        fixedPath.pop_back();

        u_int64 headPos = _diskEntity->root();

        while (!fixedPath.empty()) {

            auto part = fixedPath.front();

            fixedPath.pop_front();

            while (headPos != UNDEFINED) {
                auto inode = _diskEntity->fileINodeAt(headPos);
                if (inode.name == part) break;
                headPos = inode.next;
            }

            assert(
                    headPos != UNDEFINED,
                    "FSController::getFilePos",
                    "目标路径部分不存在：" + part
            );

            headPos = IByteable::fromBytes<u_int64>(_diskEntity->fileAt(headPos)->data);

        }

        while (headPos != UNDEFINED) {
            auto inode = _diskEntity->fileINodeAt(headPos);
            if (inode.name == last) break;
            headPos = inode.next;
        }

        assert(
                headPos != UNDEFINED,
                "FSController::getFilePos",
                "目标项目不存在：" + last
        );

        return headPos;
    }

    std::string FSController::getTitle() const {
        std::stringstream builder;
        builder << _diskEntity->getPath() << " @";
        if (role == INode::Admin) {
            builder << "超级用户";
        } else if (onlineUser != nullptr) {
            builder << onlineUser->username;
        } else {
            builder << "未登录";
        }
        return builder.str();
    }

    u_int64 FSController::createDir(const std::list<std::string> &folderPath, std::string fileName,
                                    INode::PermissionGroup permission) {

        assertLogin();

        auto dirFiles = getDir(fixPath(folderPath));

        assert(
                !std::any_of(
                        dirFiles.begin(),
                        dirFiles.end(),
                        [&fileName](const INode &it) -> bool {
                            return it.name == fileName;
                        }
                ),
                "FSController::createDir",
                "当前目录下已存在相同文件名的项目！"
        );

        // 创建并添加新的文件夹
        INode newFolderINode{fileName, 8, permission, INode::FOLDER_TYPE, 0, UNDEFINED};
        auto createPos = _diskEntity->addFile(newFolderINode, IByteable::toBytes(UNDEFINED));

        assert(createPos != UNDEFINED, "FSController::createDir", "文件夹创建失败：当前系统已没有足够空间！");

        // 将新的文件夹链接到当前目录下
        u_int64 head;

        if (folderPath.empty()) {
            // root
            head = _diskEntity->root();
            if (head == UNDEFINED) {
                // root 为空
                _diskEntity->setRoot(createPos);
                return createPos;
            }
        } else {
            // 获取目录
            auto folderPos = this->getFilePos(folderPath);
            auto folderFile = _diskEntity->fileAt(folderPos);
            assert(folderFile->inode.getType() == INode::Folder, "FSController::createDir", "目标不为文件夹");
            head = IByteable::fromBytes<u_int64>(folderFile->data);
            if (head == UNDEFINED) {
                // 目录为空
                folderFile->data = IByteable::toBytes(createPos);
                _diskEntity->updateWithoutSizeChange(folderPos, *folderFile);
                return createPos;
            }
        }

        u_int64 next = _diskEntity->fileAt(head)->inode.next;

        while (next != UNDEFINED) {
            head = next;
            next = _diskEntity->fileAt(head)->inode.next;
        }

        _diskEntity->updateNextAt(head, createPos);
        return createPos;
    }

    std::list<INode> FSController::getDir(const std::list<std::string> &filePath) {

        u_int64 head;

        if (filePath.empty()) {
            head = _diskEntity->root();
        } else {
            auto targetFolder = getFilePos(filePath);
            assert(_diskEntity->fileINodeAt(targetFolder).getType() == INode::Folder);
            head = IByteable::fromBytes<u_int64>(_diskEntity->fileAt(targetFolder)->data);
        }

        std::list<INode> res{};

        while (head != UNDEFINED) {
            INode iNode = _diskEntity->fileINodeAt(head);
            res.push_back(iNode);
            head = iNode.next;
        }

        return res;
    }

    INode FSController::getINodeByPath(const std::list<std::string> &folderPath) {
        return _diskEntity->fileINodeAt(getFilePos(folderPath));
    }

    u_int64
    FSController::createFile(const std::list<std::string> &_folderPath, std::string fileName, const ByteArray &data,
                             INode::PermissionGroup permission) {

        assertLogin();

        auto dirFiles = getDir(fixPath(_folderPath));

        assert(
                !std::any_of(
                        dirFiles.begin(),
                        dirFiles.end(),
                        [&fileName](const INode &it) -> bool {
                            return it.name == fileName;
                        }
                ),
                "FSController::createFile",
                "当前目录下已存在相同文件名的项目！"
        );

        bool intoRoot = false;

        u_int64 targetFolder;

        if (_folderPath.empty()) {
            intoRoot = true;
        } else {
            targetFolder = getFilePos(_folderPath);
        }

        auto newFilePos = _diskEntity->addFile(
                INode{
                        std::move(fileName),
                        data.size(),
                        permission,
                        std::byte{0},
                        0,
                        UNDEFINED
                },
                data
        );

        assert(newFilePos != UNDEFINED, "FSController::createFile", "磁盘已满！");

        if (intoRoot) {

            auto headPos = _diskEntity->root();

            if (headPos == UNDEFINED) {
                _diskEntity->setRoot(newFilePos);
            } else {
                INode inode = _diskEntity->fileINodeAt(headPos);
                while (inode.next != UNDEFINED) {
                    headPos = inode.next;
                    inode = _diskEntity->fileINodeAt(headPos);
                }
                _diskEntity->updateNextAt(headPos, newFilePos);
            }

        } else {
            auto folder = _diskEntity->fileAt(targetFolder);

            auto headPos = IByteable::fromBytes<u_int64>(folder->data);

            if (headPos == UNDEFINED) {
                folder->data = IByteable::toBytes(newFilePos);
                _diskEntity->updateWithoutSizeChange(targetFolder, *folder);
            } else {
                INode inode = _diskEntity->fileINodeAt(headPos);
                while (inode.next != UNDEFINED) {
                    headPos = inode.next;
                    inode = _diskEntity->fileINodeAt(headPos);
                }
                _diskEntity->updateNextAt(headPos, newFilePos);
            }
        }

        return newFilePos;
    }

    void FSController::printStructure(std::ostream &os) {
        for (const auto &item: _diskEntity->getAll()) {
            if (item.type == NodeType::File) {
                os << item.ptr.file->toString(item.position);
            } else if (item.type == NodeType::Empty) {
                os << item.ptr.empty->toString(item.position);
            }
            os << endl;
        }
    }

    void FSController::removeFile(const std::list<std::string> &_filePath, bool ignoreFolder, std::ostream *os) {

        assertLogin();

        if (os != nullptr) {
            *os << "删除： " << pathStr(_filePath, false) << endl;
        }

        auto filePath = _filePath;

        assert(
                !filePath.empty(),
                "FSController::removeFile",
                "无法删除根目录！"
        );

        assert(
                _diskEntity->fileINodeAt(getFilePos(filePath)).assertPermission(INode::Edit, role),
                "FSController::removeDir",
                "没有足够的权限！"
        );

        auto fileName = filePath.back();
        filePath.pop_back();

        if (filePath.empty()) {

            u_int64 lastFilePos = _diskEntity->root();

            INode headFileINode = _diskEntity->fileINodeAt(lastFilePos);

            if (headFileINode.name == fileName) {

                if (!ignoreFolder) {
                    assert(headFileINode.getType() == INode::UserFile, "FSController::removeFile", "目标项目不为文件0");
                }

                _diskEntity->setRoot(headFileINode.next);
                _diskEntity->removeFileAt(lastFilePos);

                return;
            }

            u_int64 thisFilePos = headFileINode.next;

            INode thisFileINode;

            while (thisFilePos != UNDEFINED) {
                thisFileINode = _diskEntity->fileINodeAt(thisFilePos);
                if (thisFileINode.name == fileName) break;

                lastFilePos = thisFilePos;
                thisFilePos = thisFileINode.next;
            }

            assert(thisFilePos != UNDEFINED, "FSController::removeFile", "目标文件不存在");
            if (!ignoreFolder) {
                assert(thisFileINode.getType() == INode::UserFile, "FSController::removeFile", "目标项目不为文件1");
            }

            _diskEntity->updateNextAt(lastFilePos, thisFileINode.next);
            _diskEntity->removeFileAt(thisFilePos);

            return;

        } else {

            auto dirPos = getFilePos(filePath);

            auto dirINode = _diskEntity->fileINodeAt(dirPos);

            assert(dirINode.getType() == INode::Folder);

            auto dirFolderFile = _diskEntity->fileAt(dirPos);

            auto lastFilePos = IByteable::fromBytes<u_int64>(dirFolderFile->data);

            INode headFileINode = _diskEntity->fileINodeAt(lastFilePos);

            if (headFileINode.name == fileName) {

                if (!ignoreFolder) {
                    assert(headFileINode.getType() == INode::UserFile, "FSController::removeFile", "目标项目不为文件2");
                }

                dirFolderFile->data = IByteable::toBytes(headFileINode.next);

                _diskEntity->updateWithoutSizeChange(dirPos, *dirFolderFile);
                _diskEntity->removeFileAt(lastFilePos);

                return;
            }

            u_int64 thisFilePos = headFileINode.next;

            INode thisFileINode;

            while (thisFilePos != UNDEFINED) {
                thisFileINode = _diskEntity->fileINodeAt(thisFilePos);
                if (thisFileINode.name == fileName) break;

                lastFilePos = thisFilePos;
                thisFilePos = thisFileINode.next;
            }

            assert(thisFilePos != UNDEFINED, "FSController::removeFile", "目标文件不存在");
            if (!ignoreFolder) {
                assert(thisFileINode.getType() == INode::UserFile, "FSController::removeFile", "目标项目不为文件3");
            }

            _diskEntity->updateNextAt(lastFilePos, thisFileINode.next);
            _diskEntity->removeFileAt(thisFilePos);

            return;
        }
    }

    void FSController::removeDir(const std::list<std::string> &_folderPath, std::ostream *os) {

        assertLogin();

        auto folderPath = fixPath(_folderPath);

        assert(!folderPath.empty(), "FSController::removeDir", "无法删除根目录");

        auto folderPos = getFilePos(folderPath);

        assert(folderPos != UNDEFINED, "FSController::removeDir");

        auto inode = _diskEntity->fileINodeAt(folderPos);

        assert(inode.assertPermission(INode::Edit, role), "FSController::removeDir", "没有足够的权限");

        if (inode.getType() == INode::UserFile) {
            removeFile(_folderPath, os);
        } else {
            removeDirRecursion(IByteable::fromBytes<u_int64>(_diskEntity->fileAt(folderPos)->data), _folderPath, os);
            removeFile(_folderPath, true, os);
        }
    }

    void FSController::removeDirRecursion(u_int64 headPosition, const std::list<std::string> &_folderPath,
                                          std::ostream *os) {
        std::vector<std::pair<u_int64, INode>> subs{};

        while (headPosition != UNDEFINED) {
            auto inode = _diskEntity->fileINodeAt(headPosition);
            subs.emplace_back(headPosition, inode);
            headPosition = inode.next;
        }

        for (const auto &it: subs) {
            if (it.second.getType() == INode::Folder) {
                auto folderPath = _folderPath;
                folderPath.push_back(it.second.name);
                removeDirRecursion(IByteable::fromBytes<u_int64>(_diskEntity->fileAt(it.first)->data), folderPath, os);
            }
        }

        for (auto &sub: std::ranges::reverse_view(subs)) {
            auto filePath = _folderPath;
            filePath.push_back(sub.second.name);
            removeFile(filePath, true, os);
        }
    }

    void FSController::changeRole(INode::Role targetRole, const std::string &password) {
        if (targetRole == INode::Admin) {
            assert(_diskEntity->assertSuperUser(password), "FSController::changeRole", "超级用户密码错误");
        }

        role = targetRole;
    }

    FSController::EditSession FSController::editFile(const std::list<std::string> &filePath) {

        assertLogin();

        u_int64 filePos;

        try {
            filePos = getFilePos(filePath);
        } catch (Error &) {
            this->createFile(filePath, ByteArray(), INode::OpenPermission);
            filePos = getFilePos(filePath);
        }

        FileNode *targetFile = _diskEntity->fileAt(filePos);

        assert(
                targetFile->inode.getType() == INode::UserFile,
                "FSController::editFile",
                "目标项目不为文件"
        );

        assert(
                targetFile->inode.assertPermission(INode::Edit, role),
                "FSController::editFile",
                "没有足够的权限"
        );

        assert(
                !targetFile->inode.isEditing(),
                "FSController::editFile",
                "该文件正在被其他用户写"
        );

        targetFile->inode.openCounter = 1;

        _diskEntity->updateWithoutSizeChange(filePos, *targetFile);

        return {
                targetFile->data,
                targetFile->inode,
                filePath,
                [this](
                        const auto &_0,
                        const auto &_1,
                        const auto &_2
                ) -> bool {
                    try {
                        auto res = updateFile(_0, _1, _2);
                        releaseWriteLock(_2);
                        return res;
                    } catch (Error &e) {
                        releaseWriteLock(_2);
                        throw e;
                    }
                },
                [this](
                        const auto &_0
                ) {
                    releaseWriteLock(_0);
                }
        };

    }

    u_int64 FSController::createFile(
            const std::list<std::string> &_filePath,
            const ByteArray &data,
            INode::PermissionGroup permission) {

        assertLogin();

        auto path = _filePath;

        auto fileName = path.back();
        path.pop_back();

        return createFile(path, fileName, data, permission);
    }

    bool
    FSController::updateFile(const ByteArray &newData, const INode &oldINode, const std::list<std::string> &oldPath) {
        assertLogin();
        removeFile(oldPath);
        return createFile(oldPath, newData, oldINode.permission) != UNDEFINED;
    }

    void FSController::releaseWriteLock(const std::list<std::string> &oldPath) {
        auto filePos = getFilePos(oldPath);
        auto file = _diskEntity->fileAt(filePos);
        file->inode.openCounter = 0;
        _diskEntity->updateWithoutSizeChange(filePos, *file);
    }

    void
    FSController::setFilePermission(const std::list<std::string> &_filePath, INode::PermissionGroup permissionGroup) {
        assertLogin();
        assert(role == INode::Admin, "FSController::setFilePermission", "需要管理员身份");

        auto filePos = getFilePos(_filePath);

        assert(filePos != UNDEFINED, "FSController::setFilePermission", "目标文件不存在");

        auto file = _diskEntity->fileAt(filePos);

        file->inode.permission = permissionGroup;

        _diskEntity->updateWithoutSizeChange(filePos, *file);
    }

    std::string FSController::getScript(const std::list<std::string> &_filePath) {
        assertLogin();
        auto filePos = getFilePos(_filePath);
        assert(filePos != UNDEFINED, "FSController::getScript", "目标文件不存在");
        auto file = _diskEntity->fileAt(filePos);
        assert(file->inode.assertPermission(INode::Execute, role), "FSController::getScript", "没有足够的权限");

        return std::string{reinterpret_cast<char *>(file->data.data()), file->data.flatSize()};
    }

    std::list<std::string> FSController::getUserMapPath() {
        return {"user.map"};
    }

    void FSController::format(std::string adminPassword) {
        assert(role == INode::Admin, "FSController::format", "权限不足");
        _diskEntity->format(adminPassword);
        changeRole(INode::Admin, adminPassword);
        createFile(getUserMapPath(), {}, INode::AdminOnlyPermission);
    }

    UserTable FSController::getUsers() {
        return *UserTable::parse(_diskEntity->fileAt(getFilePos(getUserMapPath()))->data);
    }

    bool FSController::setUsers(UserTable users) {
        assert(role == INode::Admin, "FSController::setUsers", "权限不足");
        return editFile(getUserMapPath()).assignEditFinish(users.toBytes());
    }

    void FSController::registerUser(std::string username, const std::string &password) {
        assert(role == INode::Admin, "FSController::registerUser", "权限不足");
        auto users = getUsers();
        assert(users.pushUser(UserItem(std::move(username), password)), "FSController::registerUser",
               "已存在相同用户名的用户！");
        assert(setUsers(users), "FSController::registerUser", "剩余磁盘大小不足以添加新用户！");
    }

    bool FSController::login(std::string username, std::string password) {
        changeRole(INode::User);
        onlineUser = getUsers().login(std::move(username), std::move(password));
        return onlineUser != nullptr;
    }

    void FSController::assertLogin() {
        assert(role == INode::Admin || onlineUser != nullptr, "FSController::assertLogin", "未登录！");
    }

    std::string FSController::cat(const std::list<std::string> &_filePath) {
        assertLogin();
        auto file = _diskEntity->fileAt(getFilePos(_filePath));
        assert(file->inode.assertPermission(INode::Read, role), "FSController::cat", "没有足够的权限！");
        return std::string{reinterpret_cast<const char*>(file->data.data()), file->data.size()};
    }

} // FileSystem