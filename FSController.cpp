//
// Created by actre on 12/2/2023.
//

#include "FSController.h"

#include <utility>

namespace FileSystem {

    bool FSController::good() const {
        return this->_diskEntity != nullptr;
    }

    void FSController::create(u_int64 size, std::string path, const std::string &root_password) {
        this->_diskEntity = new DiskEntity{size, std::move(path), root_password};
    }

    void FSController::setPath(std::string path) {
        this->_diskEntity = new DiskEntity{std::move(path)};
    }

    u_int64 FSController::getFileAtPath(u_int64 position, const std::string &fileName, bool isPathStart) {
        if (position == UNDEFINED) return UNDEFINED;

        u_int64 targetFilePos;

        if (!isPathStart) {
            targetFilePos = position;
        } else {
            INode fileINode = _diskEntity->fileINodeAt(position);
            if (fileINode.getType() != INode::Folder) {
                return UNDEFINED;
            }
            FileNode *pathFile = _diskEntity->fileAt(position);

            targetFilePos = IByteable::fromBytes<u_int64>(pathFile->data);
        }

        while (targetFilePos != UNDEFINED) {
            auto inode = _diskEntity->fileINodeAt(targetFilePos);
            if (std::string{inode.name, std::to_integer<unsigned long>(inode.nameLength)} == fileName)
                break;
            targetFilePos = inode.next;
        }

        if (targetFilePos == UNDEFINED) return UNDEFINED;

        return targetFilePos;
    }


    u_int64 FSController::getFilePos(std::list<std::string> filePath) {
        filePath.pop_front();

        u_int64 targetPos = _diskEntity->root();
        bool isRoot = true;

        for (const auto &it: filePath) {
            if (targetPos == UNDEFINED) break;
            targetPos = getFileAtPath(targetPos, it, isRoot);
            isRoot = false;
        }

        if (isRoot)
            throw Error("FSController::getFilePos", "尝试访问根目录文件");

        return targetPos;
    }

    std::list<std::pair<u_int64, INode>> FSController::getDir(std::list<std::string> folderPath) {

        u_int64 targetPath;

        if (folderPath.empty()) {
            targetPath = _diskEntity->root();
        } else {
            u_int64 pathPos = getFilePos(std::move(folderPath));
            if (pathPos == UNDEFINED) return {};
            auto pathFile = _diskEntity->fileAt(pathPos);
            assert(pathFile->inode.getType() == INode::Folder, "FSController::getDir");
            targetPath = IByteable::fromBytes<u_int64>(pathFile->data);
        }

        std::list<std::pair<u_int64, INode>> res{};

        while (targetPath != UNDEFINED) {
            cout << targetPath << endl;
            auto inode = _diskEntity->fileINodeAt(targetPath);
            cout << "inode.next: " << inode.next << endl;
            res.emplace_back(targetPath, inode);
            targetPath = inode.next;
        }

        return res;
    }

    std::string FSController::getPath() {
        return _diskEntity->getPath();
    }

    u_int64 FSController::createDir(std::list<std::string> folderPath, std::string fileName) {

        INode newFolderINode{fileName, 8, std::byte{0xFF}, INode::FOLDER_TYPE, 0, UNDEFINED};
        auto createPos = _diskEntity->addFile(newFolderINode, IByteable::toBytes(UNDEFINED));

        if (createPos == UNDEFINED) {
            throw Error{"FSController::createDir", "文件夹创建失败：当前系统已没有足够空间！"};
        }

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
            auto folderPos = this->getFilePos(folderPath);
            auto folderFile = _diskEntity->fileAt(folderPos);
            assert(folderFile != nullptr);
            assert(folderFile->inode.getType() == INode::Folder, "FSController::createDir", "目标不为文件夹");
            assert(folderFile->data.size() == sizeof(u_int64));
            head = IByteable::fromBytes<u_int64>(folderFile->data);
            if (head == UNDEFINED) {
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

} // FileSystem