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
                    std::format("目标路径部分不存在：{}", part)
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
                std::format("目标项目不存在：{}", last)
        );

        return headPos;
    }

    std::string FSController::getDiskTitle() const {
        return _diskEntity->getPath();
    }

    u_int64 FSController::createDir(const std::list<std::string> &folderPath, std::string fileName) {

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
        INode newFolderINode{fileName, 8, std::byte{0xFF}, INode::FOLDER_TYPE, 0, UNDEFINED};
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

} // FileSystem