//
// Created by actre on 12/2/2023.
//

#include "FileSystemConnector.h"

namespace FileSystem {

    FileSystemConnector::FileSystemConnector(DiskEntity &diskEntity) : _diskEntity(diskEntity) {}

    u_int64 FileSystemConnector::getFileAtPath(u_int64 position, const std::string &fileName, bool isPathStart) {
        if (position == UNDEFINED) return UNDEFINED;

        u_int64 targetFilePos;

        if (!isPathStart) {
            targetFilePos = position;
        } else {
            INode fileINode = _diskEntity.fileINodeAt(position);
            if (fileINode.getType() != INode::Path) {
                return UNDEFINED;
            }
            FileNode *pathFile = _diskEntity.fileAt(position);

            targetFilePos = IByteable::fromBytes<u_int64>(pathFile->data);
        }

        while (targetFilePos != UNDEFINED) {
            auto inode = _diskEntity.fileINodeAt(targetFilePos);
            if (std::string{inode.name, std::to_integer<unsigned long>(inode.nameLength)} == fileName)
                break;
            targetFilePos = inode.next;
        }

        if (targetFilePos == UNDEFINED) return UNDEFINED;

        return targetFilePos;
    }


    u_int64 FileSystemConnector::getFilePos(const std::string &_filePath) {
        auto filePath = checkPath(_filePath);
        assert(filePath[0] == '/', "FileSystemConnector::getFilePos");

        std::list<std::string> pathParts = splitString(filePath, '/');

        pathParts.pop_front();

        u_int64 targetPos = _diskEntity.root();
        bool isRoot = true;

        for (const auto &it: pathParts) {
            if (targetPos == UNDEFINED) break;
            targetPos = getFileAtPath(targetPos, it, isRoot);
            isRoot = false;
        }

        if (isRoot)
            throw Error("FileSystemConnector::getFilePos", "尝试访问根目录文件");

        return targetPos;
    }

    std::list<std::pair<u_int64, INode>> FileSystemConnector::getDir(const std::string &_filePath) {
        auto filePath = checkPath(_filePath);
        u_int64 targetPath;
        if (filePath == "/")
            targetPath = _diskEntity.root();
        else {
            u_int64 pathPos = getFilePos(filePath);
            if (pathPos == UNDEFINED) return {};
            auto pathFile = _diskEntity.fileAt(pathPos);
            assert(pathFile->inode.getType() == INode::Path, "FileSystemConnector::getDir");
            targetPath = IByteable::fromBytes<u_int64>(pathFile->data);
        }

        std::list<std::pair<u_int64, INode>> res{};

        while (targetPath != UNDEFINED) {
            auto inode = _diskEntity.fileINodeAt(targetPath);
            res.emplace_back(targetPath, inode);
            targetPath = inode.next;
        }

        return res;
    }

} // FileSystem