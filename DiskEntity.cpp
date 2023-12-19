//
// Created by actre on 11/23/2023.
//

#include "DiskEntity.h"

#include <utility>
#include <fstream>
#include "FileNode.h"
#include "EmptyNode.h"
#include "SHA256.h"

namespace FileSystem {

    void DiskEntity::format(u_int64 diskSize, const std::string &root_password) {

        ByteArray prefix = ByteArray()

                // 文件系统标识
                .append(reinterpret_cast<const std::byte *>("SakulinF"), 8)

                        // 磁盘大小
                .append(IByteable::toBytes(diskSize))

                        // Root 根目录头文件地址
                .append(IByteable::toBytes(UNDEFINED))

                        // 空闲链表头地址
                .append(IByteable::toBytes(FILE_INDEX_START))

                        // 超级用户密码
                .append(reinterpret_cast<const std::byte *>(Ly::Sha256::getInstance().getHexMessageDigest(
                        root_password).data()), 32)

                        // 文件数据（初始时全空）
                .append(EmptyNode(UNDEFINED, UNDEFINED, diskSize - FILE_INDEX_START, UNDEFINED, UNDEFINED).toBytes());

        _fileLinker.doWithFileO(0, 0, [&](std::ofstream &it) {
            it.write(reinterpret_cast<const char *>(prefix.data()), (std::streamsize) prefix.size());
        });
    }

    DiskEntity::DiskEntity(u_int64 size, std::string path, const std::string &root_password) : _fileLinker(
            std::move(path)) {
        _fileLinker.create();
        _fileLinker.resize(size);
        format(size, root_password);
    }

    DiskEntity::DiskEntity(std::string path) : _fileLinker(path) {
        checkFormat();
    }


    u_int64 DiskEntity::addFile(const INode &iNode, ByteArray byteArray) {

        FileNode targetFile = FileNode{0, 0, iNode, 0, std::move(byteArray)};

        u_int64 lastEmptyNodeNextEmptyPosWritePos = EMPTY_START;

        auto thisEmptyNodePos = getFirstEmpty();

        auto emptyNode = emptyAt(thisEmptyNodePos);

        while (emptyNode != nullptr) {

            if (emptyNode->emptySize >= targetFile.mainSize())
                break;

            lastEmptyNodeNextEmptyPosWritePos = thisEmptyNodePos + NEXT_NODE_START;

            thisEmptyNodePos = emptyNode->nextEmpty;

            emptyNode = emptyAt(thisEmptyNodePos);
        }

        if (emptyNode == nullptr)
            return UNDEFINED;

        u_int64 emptySize = emptyNode->emptySize - targetFile.mainSize();

        if (emptySize < EmptyNode::MIN_REQUIRE_SIZE) {

            // 节点结构不变

            targetFile.expansionSize = emptySize;
            targetFile.lastNode = emptyNode->lastNode;
            targetFile.nextNode = emptyNode->nextNode;
            assert(targetFile.mainSize() == emptyNode->emptySize, "DiskEntity::addFile",
                   "扩容后文件大小不等于空容量大小");

            auto nextEmptyPos = emptyNode->nextEmpty;

            if (thisEmptyNodePos == getFirstEmpty()) {
                updateFirstEmpty(nextEmptyPos);
            }

            _fileLinker.write(lastEmptyNodeNextEmptyPosWritePos, 0, IByteable::toBytes(nextEmptyPos));
            _fileLinker.write(thisEmptyNodePos, 0, targetFile.toBytes());
        } else {
            EmptyNode node = EmptyNode{thisEmptyNodePos, emptyNode->nextNode, emptySize, emptyNode->lastEmpty,
                                       emptyNode->nextEmpty};
            u_int64 newEmptyNodePos = thisEmptyNodePos + targetFile.mainSize();

            // 设置下一个节点的 上一个节点位置
            auto nextNode = emptyNode->nextNode;

            if (nextNode != UNDEFINED) {
                _fileLinker.write(nextNode, FileSystem::LAST_NODE_START, IByteable::toBytes(newEmptyNodePos));
            }

            targetFile.lastNode = emptyNode->lastNode;
            targetFile.nextNode = newEmptyNodePos;

            if (thisEmptyNodePos == getFirstEmpty()) {
                updateFirstEmpty(newEmptyNodePos);
            }

            _fileLinker.write(lastEmptyNodeNextEmptyPosWritePos, 0, IByteable::toBytes(newEmptyNodePos));
            _fileLinker.write(newEmptyNodePos, 0, node.toBytes());
            _fileLinker.write(thisEmptyNodePos, 0, targetFile.toBytes());
        }

        return thisEmptyNodePos;
    }

    EmptyNode *DiskEntity::emptyAt(u_int64 position) {
        if (position == UNDEFINED) return nullptr;

        EmptyNode *node;

        _fileLinker.doWithFileI(position, 0, [&](std::ifstream &it) {
            node = EmptyNode::parse(it);
        });

        return node;
    }

    FileNode *DiskEntity::fileAt(u_int64 position) {
        if (position == UNDEFINED) return nullptr;
        FileNode *node = nullptr;
        _fileLinker.doWithFileI(position, 0, [&node](auto &it) { node = FileNode::parse(it); });
        return node;
    }

    void DiskEntity::removeFileAt(u_int64 position) {

        FileNode *file = fileAt(position);
        if (file == nullptr) return;

        u_int64 fileSize = file->mainSize();
        u_int64 emptyPos;
        EmptyNode *empty;

        auto lastNodeStream = _fileLinker.getFileInput(file->lastNode, 0);
        auto lastNodeTypeStr = ByteArray().read(*lastNodeStream, 4, false);
        lastNodeStream->close();


        auto nextNodeStream = _fileLinker.getFileInput(file->nextNode, 0);
        auto nextNodeTypeStr = ByteArray().read(*nextNodeStream, 4, false);
        nextNodeStream->close();

        bool lastNodeTypeIsEmpty = FileSystem::Empty == FileSystem::getType(lastNodeTypeStr);

        bool nextNodeTypeIsEmpty = FileSystem::Empty == FileSystem::getType(nextNodeTypeStr);

        if (lastNodeTypeIsEmpty && nextNodeTypeIsEmpty) { // 11

            emptyPos = file->lastNode;
            empty = emptyAt(emptyPos);

            auto nextEmpty = emptyAt(file->nextNode);

            assert(empty != nullptr && nextEmpty != nullptr, "DiskEntity::removeFileAt", "1");

            // 设置下一个空节点的 上一个空节点位置
            u_int64 nextEmptyNextEmptyPos = nextEmpty->nextEmpty;

            if (nextEmptyNextEmptyPos != UNDEFINED) {
                _fileLinker.write(nextEmpty->nextEmpty, EmptyNode::LAST_EMPTY_START, IByteable::toBytes(emptyPos));
            }

            // 设置下一个节点的 上一个节点位置
            u_int64 nextNodePos = nextEmpty->nextNode;

            if (nextNodePos != UNDEFINED) {
                _fileLinker.write(nextNodePos, FileSystem::LAST_NODE_START, IByteable::toBytes(emptyPos));
            }

            // 设置这个节点的 下一个节点位置
            empty->nextNode = nextNodePos;

            // 设置这个节点的 下一个空节点位置
            empty->nextEmpty = nextEmptyNextEmptyPos;

            // 重新设置这个空节点的大小
            empty->emptySize = empty->emptySize + fileSize + nextEmpty->emptySize;

            // 无需检查 FIRST_EMPTY
            _fileLinker.write(emptyPos, 0, empty->toBytes());

        } else if (lastNodeTypeIsEmpty) { // 10

            emptyPos = file->lastNode;
            empty = emptyAt(emptyPos);

            u_int64 nextNodePos = file->nextNode;

            // 设置下一个节点的 上一个节点位置
            if (nextNodePos != UNDEFINED) {
                _fileLinker.write(nextNodePos, FileSystem::LAST_NODE_START, IByteable::toBytes(emptyPos));
            }

            // 设置这个空节点的 下一个节点位置
            empty->nextNode = file->nextNode;

            // 重新设置这个空节点的大小
            empty->emptySize += fileSize;

            // 无需检查 FIRST_EMPTY
            _fileLinker.write(emptyPos, 0, empty->toBytes());

        } else if (nextNodeTypeIsEmpty) { // 01

            emptyPos = position;
            u_int64 oldEmptyPos = file->nextNode;
            empty = emptyAt(oldEmptyPos);
            u_int64 nextEmptyPos = empty->nextEmpty;
            u_int64 lastEmptyPos = empty->lastEmpty;
            u_int64 nextNodePos = empty->nextNode;

            // 设置下一个空节点的 上一个空节点位置
            if (nextEmptyPos != UNDEFINED) {
                _fileLinker.write(nextEmptyPos, EmptyNode::LAST_EMPTY_START, IByteable::toBytes(emptyPos));
            }

            // 设置下一个节点的 上一个节点位置
            if (nextNodePos != UNDEFINED) {
                _fileLinker.write(nextEmptyPos, FileSystem::LAST_NODE_START, IByteable::toBytes(emptyPos));
            }

            // 设置上一个空节点的 下一个空节点位置
            if (lastEmptyPos != UNDEFINED) {
                _fileLinker.write(lastEmptyPos, EmptyNode::NEXT_EMPTY_START, IByteable::toBytes(emptyPos));
            }

            // 设置这个空节点的 上一个节点位置
            empty->lastNode = file->lastNode;

            // 重新设置这个空节点的大小
            empty->emptySize += fileSize;

            if (oldEmptyPos == getFirstEmpty()) {
                updateFirstEmpty(emptyPos);
            }

            _fileLinker.write(emptyPos, 0, empty->toBytes());

        } else { // 00

            emptyPos = position;

            bool flag = false;

            // 找到上一个、下一个空节点位置
            u_int64 lastEmptyPos = findLastEmpty(position);
            u_int64 nextEmptyPos;
            if (lastEmptyPos != UNDEFINED) {
                nextEmptyPos = emptyAt(lastEmptyPos)->nextEmpty;
            } else {
                flag = true;
                nextEmptyPos = findNextEmpty(position);
            }

            // 设置上一个空节点的 下一个空节点位置
            if (lastEmptyPos != UNDEFINED) {
                _fileLinker.write(lastEmptyPos, EmptyNode::NEXT_EMPTY_START, IByteable::toBytes(emptyPos));
            }

            // 设置下一个空节点的 上一个空节点位置
            if (nextEmptyPos != UNDEFINED) {
                _fileLinker.write(nextEmptyPos, EmptyNode::LAST_EMPTY_START, IByteable::toBytes(emptyPos));
            }

            // 配置该空节点
            empty = new EmptyNode(file->lastNode, file->nextNode, fileSize, lastEmptyPos, nextEmptyPos);

            if (flag) {
                assert(nextEmptyPos == getFirstEmpty());
                updateFirstEmpty(emptyPos);
            }

            _fileLinker.write(emptyPos, 0, empty->toBytes());
        }
    }

    u_int64 DiskEntity::root() {
        return _fileLinker.readAt<u_int64>(0, DiskEntity::ROOT_START);
    }

    u_int64 DiskEntity::findLastEmpty(u_int64 nowNode) {

        u_int64 res = nowNode;

        while (res != UNDEFINED) {

            auto fi = _fileLinker.getFileInput(res, 0);
            bool flag = FileSystem::Empty != getType(*fi);
            fi->close();

            if (!flag) break;

            res = _fileLinker.readAt<u_int64>(res, FileSystem::LAST_NODE_START);
        }

        return res;
    }

    u_int64 DiskEntity::findNextEmpty(u_int64 nowNode) {
        u_int64 res = nowNode;

        while (res != UNDEFINED) {

            auto fi = _fileLinker.getFileInput(res, 0);
            bool flag = FileSystem::Empty != getType(*fi);
            fi->close();

            if (!flag) break;

            res = _fileLinker.readAt<u_int64>(res, FileSystem::NEXT_NODE_START);
        }

        return res;
    }

    INode DiskEntity::fileINodeAt(u_int64 position) {
        INode *iNode;

        _fileLinker.doWithFileI(position, FileNode::INODE_START, [&](std::ifstream &it) {
            iNode = INode::parse(it);
        });

        return *iNode;
    }

    void DiskEntity::checkFormat() {

        bool sizeGood;

        std::string prefix;
        u_int64 stateSize;
        auto fileSize = _fileLinker.size();

        _fileLinker.doWithFileI(0, 0, [&](std::ifstream &it) {
            prefix = std::string{reinterpret_cast<const char *>(ByteArray().read(it, 8, false).data()), 8};

            stateSize = IByteable::fromBytes<u_int64>(ByteArray().read(it, 8, false));
            sizeGood = stateSize == fileSize;
        });

        assert(prefix == "SakulinF", "DiskEntity::checkFormat", "系统声明错误：" + prefix);

        assert(sizeGood, "DiskEntity::checkFormat",
               "大小不相等：文件系统声明 " + std::to_string(stateSize) + " 与 实际大小 " + std::to_string(fileSize));
    }

    std::string DiskEntity::getPath() const {
        return _fileLinker.path;
    }

    void DiskEntity::setRoot(u_int64 pos) {
        _fileLinker.write(0, DiskEntity::ROOT_START, IByteable::toBytes(pos));
    }

    void DiskEntity::updateWithoutSizeChange(u_int64 originLoc, FileNode &newFile) {

        auto oldFile = fileAt(originLoc);

        assert(oldFile->inode.size == newFile.inode.size, "DiskEntity::updateWithoutSizeChange",
               "旧文件与新文件的大小不同");

        newFile.setExpansionSize(oldFile->expansionSize);

        assert(oldFile->mainSize() == newFile.mainSize(), "DiskEntity::updateWithoutSizeChange",
               "旧文件与新文件扩容后的实际大小不同");

        _fileLinker.write(0, originLoc, newFile.toBytes());
    }

    void DiskEntity::updateNextAt(u_int64 originLoc, u_int64 newNext) {
        auto it = fileINodeAt(originLoc);

        it.next = newNext;

        _fileLinker.write(originLoc, FileNode::INODE_START, it.toBytes());
    }

    void DiskEntity::updateFirstEmpty(u_int64 firstEmpty) {
        _fileLinker.write(0, DiskEntity::EMPTY_START, IByteable::toBytes(firstEmpty));
    }

    u_int64 DiskEntity::getFirstEmpty() {
        return _fileLinker.readAt<u_int64>(0, DiskEntity::EMPTY_START);
    }

    NodePtr DiskEntity::nodeAt(u_int64 position) {

        auto *input = _fileLinker.getFileInput(position, 0);

        NodePtr ptr{};

        ptr.type = getType(*input);

        input->close();

        ptr.position = position;

        if (ptr.type == FileSystem::File) {
            ptr.ptr.file = fileAt(position);
        }

        if (ptr.type == FileSystem::Empty) {
            ptr.ptr.empty = emptyAt(position);
        }

        return ptr;
    }

    std::list<NodePtr> DiskEntity::getAll() {
        std::list<NodePtr> res{};

        u_int64 target = FILE_INDEX_START;

        while (target != UNDEFINED) {
            auto node = nodeAt(target);
            res.push_back(node);

            if (node.type == NodeType::Empty) {
                target = node.ptr.empty->nextNode;
            } else if (node.type == NodeType::File) {
                target = node.ptr.file->nextNode;
            } else {
                throw Error{"DiskEntity::getAll", "Unknown Node Type At " + std::to_string(target)};
            }
        }

        return res;
    }

    bool DiskEntity::assertSuperUser(std::string password) {

        std::string sha256{Ly::Sha256::getInstance().getHexMessageDigest(password).data(), 32};

        std::istream *input = _fileLinker.getFileInput(DiskEntity::SUPERUSER_PASSWORD_START, 0);

        std::string r{reinterpret_cast<char *>(ByteArray().read(*input, 32, false).data()), 32};

        return r == sha256;
    }


} // FileSystem