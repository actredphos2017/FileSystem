//
// Created by actre on 11/23/2023.
//

#ifndef FILESYSTEM_FILENODE_H
#define FILESYSTEM_FILENODE_H

#include <cstddef>
#include <string>
#include <utility>
#include "Utils.h"

namespace FileSystem {

    const int TYPE_ROOT = 0;

    /**
     * 存储格式： | 文件标识 4 字节 | 上一节点位置 8 字节 | 下一节点位置 8 字节 | inode 索引节点 | 系统需要强制扩容大小 8 字节 | 数据 |
     * inode：
     *      文件名称长度         1 字节
     *      文件名称            动态，最多 255 字节
     *      文件大小            8 字节
     *      权限信息            1 字节
     *      类型信息            1 字节
     *      文件打开计数器       4 字节
     *      下一个同级文件地址    8 字节
     *
     * 运行时额外标识（用户内存）
     *      文件指针
     *      文件位置
     *
     *
     * 为保证空闲节点正常工作，文件至少应需要 16 字节
     *
     */

    struct INode : IByteable {
        std::byte nameLength{};
        const char *name{};
        u_int64 size{};

        std::byte permission{};
        std::byte type{};
        int openCounter{};
        u_int64 next{};

        ByteArray toBytes() override {
            return ByteArray(nameLength)
                    .append(reinterpret_cast<const std::byte *>(name), (size_t) nameLength)
                    .append(IByteable::toBytes(size))
                    .append(permission)
                    .append(type)
                    .append(IByteable::toBytes(openCounter))
                    .append(IByteable::toBytes(next));
        }

        enum Type {
            Unknown = -1,
            UserFile = 0,
            Path = 1,
        };

        [[nodiscard]] u_int64 getSize() const;

        static INode parse(std::istream &istream);

        [[nodiscard]] Type getType() const;

    };

    class FileNode : IByteable {
    public:

        static const u_int64 INODE_START = 20;
        static const u_int64 EXPANSION_OCC = 8;

        // 创建新文件
        FileNode(u_int64 lastNode, u_int64 nextNode, INode iNode, u_int64 expansionSize, ByteArray data);

        ByteArray toBytes() override;

        [[nodiscard]] u_int64 mainSize() const;

        static FileNode *parse(std::istream &input);

        void setExpansionSize(u_int64 size);

        u_int64 lastNode;
        u_int64 nextNode;
        INode inode;
        u_int64 expansionSize;
        ByteArray data;
    };

} // FileSystem

#endif //FILESYSTEM_FILENODE_H
