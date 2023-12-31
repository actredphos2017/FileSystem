//
// Created by actre on 12/2/2023.
//

#ifndef FILESYSTEM_FSCONTROLLER_H
#define FILESYSTEM_FSCONTROLLER_H

#include "DiskEntity.h"
#include "UserTable.h"

namespace FileSystem {

    class FSController {

    public:

        class EditSession {

        public:

            EditSession(
                    ByteArray fileData,
                    INode oldINode,
                    std::list<std::string> oldPath,
                    std::function<bool(const ByteArray &, INode oldINode, std::list<std::string> oldPath)> onFinish,
                    std::function<void(std::list<std::string> oldPath)> onCancel
            );

            ByteArray getFileData();

            std::string getFileName();

            bool assignEditFinish(const ByteArray &);

            void cancelEdit();

        private:

            std::function<bool(const ByteArray &, INode oldINode, std::list<std::string> oldPath)> _onFinish;
            std::function<void(std::list<std::string> oldPath)> _onCancel;
            ByteArray _fileData;
            INode _oldINode;
            std::list<std::string> _oldPath;

        };

        [[nodiscard]] bool good() const;

        void create(u_int64 size, std::string path, const std::string &root_password);

        void setPath(std::string path);

        [[nodiscard]] std::string getTitle() const;

        u_int64 createDir(const std::list<std::string> &_folderPath, std::string fileName,
                          INode::PermissionGroup permission = INode::OpenPermission);

        u_int64 createFile(const std::list<std::string> &_folderPath, std::string fileName, const ByteArray &data,
                           INode::PermissionGroup permission = INode::OpenPermission);

        u_int64
        createFile(const std::list<std::string> &_filePath, const ByteArray &data, INode::PermissionGroup permission);

        std::list<INode> getDir(const std::list<std::string> &filePath);

        INode getINodeByPath(const std::list<std::string> &folderPath);

        void removeFile(const std::list<std::string> &_filePath, bool ignoreFolder = false, std::ostream *os = nullptr);

        void changeRole(INode::Role targetRole, const std::string &password = "");

        void printStructure(std::ostream &os);

        void removeDir(const std::list<std::string> &folderPath, std::ostream *os = nullptr);

        [[nodiscard]] EditSession editFile(const std::list<std::string> &filePath);

        bool updateFile(const ByteArray &newData, const INode &oldINode, const std::list<std::string> &oldPath);

        void releaseWriteLock(const std::list<std::string> &oldPath);

        void setFilePermission(const std::list<std::string> &_filePath, INode::PermissionGroup permissionGroup);

        std::string getScript(const std::list<std::string> &_filePath);

        void format(std::string adminPassword);

        void registerUser(std::string username, const std::string& password);

        FileSystem::UserTable getUsers();

        std::list<std::string> getUserMapPath();

        bool setUsers(UserTable users);

        bool login(std::string username, std::string password);

        std::string cat(const std::list<std::string> &_filePath);

        void assertLogin();

    private:

        INode::Role role = INode::Role::User;

        UserItem* onlineUser{};

        void
        removeDirRecursion(u_int64 position, const std::list<std::string> &_folderPath, std::ostream *os = nullptr);

        [[nodiscard]] u_int64 getFilePos(const std::list<std::string> &_filePath) const;

        DiskEntity *_diskEntity{nullptr};
    };

} // FileSystem

#endif //FILESYSTEM_FSCONTROLLER_H
