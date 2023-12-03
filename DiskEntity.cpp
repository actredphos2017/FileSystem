//
// Created by actre on 11/23/2023.
//

#include "DiskEntity.h"

#include <utility>
#include "FileNode.h"
#include "EmptyNode.h"

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

                        // 用户表地址
                .append(IByteable::toBytes(UNDEFINED))

                        // 文件数据（初始时全空）
                .append(EmptyNode(UNDEFINED, UNDEFINED, diskSize - FILE_INDEX_START, UNDEFINED, UNDEFINED).toBytes());

        _fileLinker.getFileIO() << prefix.toBytes();

        createEmptyUserTable(root_password);
    }

    DiskEntity::DiskEntity(u_int64 size, const std::string &path, const std::string &root_password) : _fileLinker(
            path) {
        _fileLinker.resize(size);
        this->format(size, root_password);
    }

    DiskEntity::DiskEntity(const std::string &path) : _fileLinker(path) {
        if (!checkFormat()) {
            throw std::exception{};
        }
    }


    u_int64 DiskEntity::addFile(const INode &iNode, ByteArray byteArray) {

        FileNode targetFile = FileNode{0, 0, iNode, 0, std::move(byteArray)};

        u_int64 lastEmptyNodeNextEmptyPosWritePos = EMPTY_START;

        auto thisEmptyNodePos = _fileLinker.readAt<u_int64>(lastEmptyNodeNextEmptyPosWritePos, 0);

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
            assert(targetFile.mainSize() == emptyNode->emptySize);
            _fileLinker.write(lastEmptyNodeNextEmptyPosWritePos, 0, IByteable::toBytes(emptyNode->nextEmpty));
            _fileLinker.write(thisEmptyNodePos, 0, targetFile.toBytes());
        } else {
            EmptyNode node = EmptyNode{thisEmptyNodePos, emptyNode->nextNode, emptySize, emptyNode->lastEmpty,
                                       emptyNode->nextEmpty};
            u_int64 newEmptyNodePos = thisEmptyNodePos + targetFile.mainSize();

            // 设置下一个节点的 上一个节点位置
            auto nextNode = emptyNode->nextNode;

            if (nextNode != UNDEFINED) {
                assert(FileSystem::Empty != getType(_fileLinker.getFileIO(nextNode)));
                _fileLinker.write(nextNode, FileSystem::LAST_NODE_START, IByteable::toBytes(newEmptyNodePos));
            }

            targetFile.lastNode = emptyNode->lastNode;
            targetFile.nextNode = newEmptyNodePos;
            _fileLinker.write(lastEmptyNodeNextEmptyPosWritePos, 0, IByteable::toBytes(newEmptyNodePos));
            _fileLinker.write(newEmptyNodePos, 0, node.toBytes());
            _fileLinker.write(thisEmptyNodePos, 0, targetFile.toBytes());
        }

        return thisEmptyNodePos;
    }

    EmptyNode *DiskEntity::emptyAt(u_int64 position) {
        if (position == UNDEFINED) return nullptr;
        return EmptyNode::parse(_fileLinker.getFileIO(position));
    }

    FileNode *DiskEntity::fileAt(u_int64 position) {
        if (position == UNDEFINED) return nullptr;
        return FileNode::parse(_fileLinker.getFileIO(position));
    }

    void DiskEntity::removeFileAt(u_int64 position) {

        FileNode *file = fileAt(position);
        if (file == nullptr) return;

        u_int64 fileSize = file->mainSize();
        u_int64 emptyPos;
        EmptyNode *empty;

        bool lastNodeTypeIsEmpty = FileSystem::Empty == getType(_fileLinker.getFileIO(file->lastNode));
        bool nextNodeTypeIsEmpty = FileSystem::Empty == getType(_fileLinker.getFileIO(file->nextNode));

        if (lastNodeTypeIsEmpty && nextNodeTypeIsEmpty) { // 11

            emptyPos = file->lastNode;
            empty = emptyAt(emptyPos);

            auto nextEmpty = emptyAt(file->nextNode);

            assert(empty != nullptr && nextEmpty != nullptr);

            // 设置下一个空节点的 上一个空节点位置
            u_int64 nextEmptyNextEmptyPos = nextEmpty->nextEmpty;

            if (nextEmptyNextEmptyPos != UNDEFINED) {
                assert(FileSystem::Empty == getType(_fileLinker.getFileIO(nextEmptyNextEmptyPos)));
                _fileLinker.write(nextEmpty->nextEmpty, EmptyNode::LAST_EMPTY_START, IByteable::toBytes(emptyPos));
            }

            // 设置下一个节点的 上一个节点位置
            u_int64 nextNodePos = nextEmpty->nextNode;

            if (nextNodePos != UNDEFINED) {
                assert(FileSystem::Empty != getType(_fileLinker.getFileIO(nextNodePos)));
                _fileLinker.write(nextNodePos, FileSystem::LAST_NODE_START, IByteable::toBytes(emptyPos));
            }

            // 设置这个节点的 下一个节点位置
            assert(FileSystem::Empty == getType(_fileLinker.getFileIO(file->lastNode)));
            empty->nextNode = nextNodePos;

            // 设置这个节点的 下一个空节点位置
            empty->nextEmpty = nextEmptyNextEmptyPos;

            // 重新设置这个空节点的大小
            empty->emptySize = empty->emptySize + fileSize + nextEmpty->emptySize;
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

            _fileLinker.write(emptyPos, 0, empty->toBytes());

        } else if (nextNodeTypeIsEmpty) { // 01

            emptyPos = position;
            empty = emptyAt(file->nextNode);
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
            empty->lastEmpty = file->lastNode;

            // 重新设置这个空节点的大小
            empty->emptySize += fileSize;

            _fileLinker.write(emptyPos, 0, empty->toBytes());

        } else { // 00

            emptyPos = position;

            // 找到上一个、下一个空节点位置
            u_int64 lastEmptyPos = findLastEmpty(position);
            u_int64 nextEmptyPos;
            if (lastEmptyPos != UNDEFINED) {
                nextEmptyPos = emptyAt(lastEmptyPos)->nextEmpty;
            } else {
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
            _fileLinker.write(emptyPos, 0, empty->toBytes());
        }
    }

    u_int64 DiskEntity::root() {
        return _fileLinker.readAt<u_int64>(0, DiskEntity::ROOT_START);
    }

    u_int64 DiskEntity::findLastEmpty(u_int64 nowNode) {

        u_int64 res = nowNode;

        while (FileSystem::Empty != getType(_fileLinker.getFileIO(res)) && res != UNDEFINED) {
            res = _fileLinker.readAt<u_int64>(res, FileSystem::LAST_NODE_START);
        }

        return res;
    }

    u_int64 DiskEntity::findNextEmpty(u_int64 nowNode) {
        u_int64 res = nowNode;

        while (FileSystem::Empty != getType(_fileLinker.getFileIO(res)) && res != UNDEFINED) {
            res = _fileLinker.readAt<u_int64>(res, FileSystem::NEXT_NODE_START);
        }

        return res;
    }

    INode DiskEntity::fileINodeAt(u_int64 position) {
        assert(File == getType(_fileLinker.getFileIO(position)));
        return INode::parse(_fileLinker.getFileIO(position + FileNode::INODE_START));
    }


} // FileSystem