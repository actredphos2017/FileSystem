//
// Created by actre on 11/23/2023.
//

#include "FileNode.h"

#include <utility>
#include <bitset>

namespace FileSystem {


    INode *INode::parse(std::istream &istream) {

        auto *res = new INode();

        auto nameSize = IByteable::fromBytes<std::byte>(ByteArray().read(istream, 1, false));

        res->name = std::string{
                reinterpret_cast<const char *>(
                        ByteArray()
                                .read(istream, static_cast<unsigned char>(nameSize), false)
                                .data()
                ),
                static_cast<unsigned char>(nameSize)
        };

        res->size = IByteable::fromBytes<u_int64>(ByteArray().read(istream, 8, false));

        res->permission = PermissionGroup::fromByte(*(ByteArray().read(istream, 1, false).data()));

        res->type = *(ByteArray().read(istream, 1, false).data());

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

    std::string INode::typeStr(INode::Type type) {
        std::string res{};
        switch (type) {
            case UserFile :
                res = "File";
                break;
            case Folder:
                res = "Folder";
                break;
            case Unknown:
                res = "Unknown";
                break;
        }
        return res;
    }

    bool INode::assertPermission(INode::PermissionType _type, INode::Role _role) {

        if (_role == Admin) {
            switch (_type) {
                case INode::PermissionType::Edit:
                    return permission.admin.editable;
                case INode::PermissionType::Read:
                    return permission.admin.readable;
                case INode::PermissionType::Execute:
                    return permission.admin.runnable;
            }
        } else {
            switch (_type) {
                case INode::PermissionType::Edit:
                    return permission.user.editable;
                case INode::PermissionType::Read:
                    return permission.user.readable;
                case INode::PermissionType::Execute:
                    return permission.user.runnable;
            }
        }

        return false;
    }

    INode::INode(std::string name, u_int64 size, INode::PermissionGroup permission, std::byte type, int openCounter,
                 u_int64 next)
            : name{std::move(name)},
              size(size),
              permission(permission),
              type(type),
              openCounter(openCounter),
              next(next) {}

    bool INode::isEditing() const {
        return openCounter > 0;
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
        assert(res.size() + expansionSize == mainSize(), "FileNode::data");
        return res;
    }

    FileNode *FileNode::parse(std::istream &input) {
        auto from = input.tellg();
        ByteArray().read(input, 4, false);
        auto _1 = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto _2 = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto _3 = *INode::parse(input);
        auto _4 = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto _5 = ByteArray().read(input, _3.size, false);
        return new FileNode(_1, _2, _3, _4, _5);
    }

    void FileNode::setExpansionSize(u_int64 size) {
        expansionSize = size;
    }

    std::string FileNode::toString(u_int64 pos) const {
        std::stringstream ss;

        ss << "===== FILE =====" << std::hex << endl;
        if (pos != 0) {
            ss << "DiskPos: " << pos << endl;
        }
        ss << "Name: " << inode.name << endl;
        ss << "Size: " << inode.size << endl;

        auto type = inode.getType();

        ss << "Type: " << INode::typeStr(type) << endl;
        ss << "Permission: " << (int) static_cast<unsigned char>(inode.permission.toByte()) << endl;
        ss << "Next: " << inode.next << endl;
        ss << "Expansion: " << expansionSize << endl;

        if (type == INode::Folder) {
            ss << "FolderHead: " << IByteable::fromBytes<u_int64>(data) << endl;
        }

        ss << std::dec;

        return ss.str();
    }

    std::byte INode::PermissionGroup::toByte() const {

        unsigned char ar = admin.readable << 6;
        unsigned char aw = admin.editable << 5;
        unsigned char ae = admin.runnable << 4;
        unsigned char ur = user.readable << 2;
        unsigned char uw = user.editable << 1;
        unsigned char ue = user.runnable << 0;

        return static_cast<std::byte> (ar | aw | ae | ur | uw | ue);
    }

    INode::PermissionGroup INode::PermissionGroup::fromByte(std::byte permission) {

        auto p = static_cast<unsigned char>(permission);

        return INode::PermissionGroup{
                {
                        static_cast<bool>((p >> 6) & 0x01),
                        static_cast<bool>((p >> 5) & 0x01),
                        static_cast<bool>((p >> 4) & 0x01)
                },
                {
                        static_cast<bool>((p >> 2) & 0x01),
                        static_cast<bool>((p >> 1) & 0x01),
                        static_cast<bool>((p >> 0) & 0x01)
                }
        };
    }
} // FileSystem