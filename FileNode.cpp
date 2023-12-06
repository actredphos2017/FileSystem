//
// Created by actre on 11/23/2023.
//

#include "FileNode.h"

#include <utility>

namespace FileSystem {


    INode *INode::parse(std::istream &istream) {

        auto *res = new INode();

        auto nameSize = IByteable::fromBytes<std::byte>(ByteArray().read(istream, 1, false));

        res->name = std::string{
                reinterpret_cast<const char *>(
                        ByteArray()
                                .read(istream, static_cast<unsigned char>(nameSize), false)
                                .toBytes()
                ),
                static_cast<unsigned char>(nameSize)
        };

        res->size = IByteable::fromBytes<u_int64>(ByteArray().read(istream, 8, false));

        res->permission = *(ByteArray().read(istream, 1, false).toBytes());

        res->type = *(ByteArray().read(istream, 1, false).toBytes());

        res->openCounter = IByteable::fromBytes<int>(ByteArray().read(istream, 4, false));

        res->next = IByteable::fromBytes<u_int64>(ByteArray().read(istream, 8, false));

        return res;
    }

    INode::Type INode::getType() const {
        switch (std::to_integer<unsigned char>(type)) {
            case 0:
                return UserFile;
            case 1:
                return Folder;
            default:
                return Unknown;
        }
    }

    u_int64 INode::getSize() const {
        return name.size() + 23;
    }

    FileNode::FileNode(u_int64 lastNode, u_int64 nextNode, INode iNode, u_int64 expansionSize, ByteArray data) :
            lastNode(lastNode),
            nextNode(nextNode),
            inode(std::move(iNode)),
            expansionSize(expansionSize),
            data(std::move(data)) {

    }

    u_int64 FileNode::mainSize() const {
        assert(data.size() == inode.size, "FileNode::mainSize");
        return FileNode::INODE_START + inode.getSize() + FileNode::EXPANSION_OCC + inode.size + expansionSize;
    }

    ByteArray FileNode::toBytes() {
        auto res = ByteArray()
                .append(reinterpret_cast<const std::byte *>("FILE"), 4)
                .append(IByteable::toBytes(lastNode))
                .append(IByteable::toBytes(nextNode))
                .append(inode.toBytes())
                .append(IByteable::toBytes(expansionSize))
                .append(data);
        assert(res.size() + expansionSize == mainSize(), "FileNode::toBytes");
        return res;
    }

    FileNode *FileNode::parse(std::istream &input) {
        auto from = input.tellg();
        ByteArray().read(input, 4, false);
        auto lastNode = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto nextNode = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto inode = INode::parse(input);
        return new FileNode(
                lastNode,
                nextNode,
                *inode,
                IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false)),
                ByteArray().read(input, inode->size, false)
        );
    }

    void FileNode::setExpansionSize(u_int64 size) {
        expansionSize = size;
    }
} // FileSystem