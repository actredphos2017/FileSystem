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

    u_int64 FSController::getFileAtFolder(u_int64 position, const std::string &fileName, bool isRoot) const {

        u_int64 targetFilePos;

        // 如果是 Root 根目录， position 应该是一个具体的位置，否则 position 指向一个文件夹，需要得到这个文件的数据部分
        if (!isRoot) {
            targetFilePos = position;
        } else {
            assert(
                    _diskEntity->fileINodeAt(position).getType() == INode::Folder,
                    "FSController::getFileAtFolder",
                    "目标项目不为文件夹"
            );
            FileNode *pathFile = _diskEntity->fileAt(position);

            targetFilePos = IByteable::fromBytes<u_int64>(pathFile->data);
        }

        while (targetFilePos != UNDEFINED) {
            auto inode = _diskEntity->fileINodeAt(targetFilePos);
            if (inode.name == fileName)
                break;
            targetFilePos = inode.next;
        }

        if (targetFilePos == UNDEFINED) return UNDEFINED;

        return targetFilePos;
    }


    u_int64 FSController::getFilePos(const std::list<std::string> &filePath) const {

        u_int64 targetPos = _diskEntity->root();

        assert(!filePath.empty(), "FSController::getFilePos", "尝试访问根目录文件 1");

        auto temp = filePath;

        auto last = *(temp.rbegin());

        temp.pop_back();

        auto inode = _diskEntity->fileINodeAt(targetPos);

        if (!temp.empty()) {

            bool notFirstLoop = false;

            for (const auto &part: temp) {
                if (notFirstLoop) {
                    assert(inode.getType() == INode::Folder, "FSController::getFilePos", "路径中存在文件 2");
                    targetPos = IByteable::fromBytes<u_int64>(_diskEntity->fileAt(targetPos)->data);
                }

                notFirstLoop = true;

                assert(targetPos != UNDEFINED, "FSController::getFilePos", "目标文件不存在 3");

                while (targetPos != UNDEFINED) {
                    if (inode.name == part) {
                        break;
                    }
                    targetPos = inode.next;
                    assert(targetPos != UNDEFINED, "FSController::getFilePos", "目标文件不存在 4");
                    inode = _diskEntity->fileINodeAt(targetPos);
                }

                assert(targetPos != UNDEFINED, "FSController::getFilePos", "目标文件不存在 5");
            }

            assert(inode.getType() == INode::Folder, "FSController::getFilePos", "路径中存在文件 6");
            targetPos = IByteable::fromBytes<u_int64>(_diskEntity->fileAt(targetPos)->data);
        }

        while (targetPos != UNDEFINED) {
            if (inode.name == last) {
                break;
            }
            targetPos = inode.next;
            assert(targetPos != UNDEFINED, "FSController::getFilePos", "目标文件不存在 7");
            inode = _diskEntity->fileINodeAt(targetPos);
        }

        assert(targetPos != UNDEFINED, "FSController::getFilePos", "目标文件不存在 8");

        return targetPos;
    }

    std::list<std::pair<u_int64, INode>> FSController::getDir(const std::list<std::string> &folderPath) {

        u_int64 targetPath;

        if (folderPath.empty()) {
            targetPath = _diskEntity->root();
        } else {
            u_int64 pathPos = getFilePos(fixPath(folderPath));

            assert(
                    _diskEntity->fileINodeAt(pathPos).getType() == INode::Folder,
                    "FSController::getDir",
                    "目标项不为文件夹"
            );

            targetPath = IByteable::fromBytes<u_int64>(_diskEntity->fileAt(pathPos)->data);
        }

        std::list<std::pair<u_int64, INode>> res{};

        while (targetPath != UNDEFINED) {
            auto inode = _diskEntity->fileINodeAt(targetPath);
            res.emplace_back(targetPath, inode);
            targetPath = inode.next;
        }

        return res;
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
                        [&fileName](const std::pair<u_int64, INode> &it) -> bool {
                            return it.second.name == fileName;
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
            assert(folderFile != nullptr);
            assert(folderFile->inode.getType() == INode::Folder, "FSController::createDir", "目标不为文件夹");
            assert(folderFile->data.size() == sizeof(u_int64));
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

    INode FSController::getINodeByPath(const std::list<std::string> &folderPath) {

        assert(!folderPath.empty(), "FSController::getINodeByPath", "尝试访问根节点");

        auto pos = getFilePos(fixPath(folderPath));

        assert(pos != UNDEFINED, "FSController::getINodeByPath", "尝试访问的项目不存在");

        return _diskEntity->fileINodeAt(pos);
    }

} // FileSystem