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
            if (fileINode.getType() != INode::Path) {
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


    u_int64 FSController::getFilePos(const std::string &_filePath) {
        auto filePath = checkPath(_filePath);
        assert(filePath[0] == '/', "FSController::getFilePos");

        std::list<std::string> pathParts = splitString(filePath, '/');

        pathParts.pop_front();

        u_int64 targetPos = _diskEntity->root();
        bool isRoot = true;

        for (const auto &it: pathParts) {
            if (targetPos == UNDEFINED) break;
            targetPos = getFileAtPath(targetPos, it, isRoot);
            isRoot = false;
        }

        if (isRoot)
            throw Error("FSController::getFilePos", "尝试访问根目录文件");

        return targetPos;
    }

    std::list<std::pair<u_int64, INode>> FSController::getDir(const std::string &_filePath) {
        auto filePath = checkPath(_filePath);
        u_int64 targetPath;
        if (filePath == "/") {
            targetPath = _diskEntity->root();
        } else {
            u_int64 pathPos = getFilePos(filePath);
            if (pathPos == UNDEFINED) return {};
            auto pathFile = _diskEntity->fileAt(pathPos);
            assert(pathFile->inode.getType() == INode::Path, "FSController::getDir");
            targetPath = IByteable::fromBytes<u_int64>(pathFile->data);
        }

        std::list<std::pair<u_int64, INode>> res{};

        while (targetPath != UNDEFINED) {
            std::cout << "Path: " << targetPath << std::endl;
            auto inode = _diskEntity->fileINodeAt(targetPath);
            res.emplace_back(targetPath, inode);
            targetPath = inode.next;
        }

        return res;
    }

} // FileSystem