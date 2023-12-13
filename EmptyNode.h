//
// Created by actre on 11/24/2023.
//

#ifndef FILESYSTEM_EMPTYNODE_H
#define FILESYSTEM_EMPTYNODE_H

#include <utility>
#include <iostream>

#include "Utils.h"

namespace FileSystem {

    /**
     * 空闲链表节点
     *
     * | 空闲标识 4 字节 | 上一节点位置 8 字节 | 下一节点位置 8 字节 | 空闲大小 8 字节 | 上一空闲节点 8 字节 | 下一个空闲节点 8 字节 |
     *
     * 空闲链表节点的标识本身需占用 44 字节
     * 如果空闲区域小于 44 字节，则强制扩容在其之前的文件
     *
     */


    class EmptyNode : IByteable {
    public:
        const static u_int64 LAST_EMPTY_START = 28;
        const static u_int64 NEXT_EMPTY_START = 36;
        const static u_int64 MIN_REQUIRE_SIZE = 44;

        EmptyNode(u_int64 lastNode, u_int64 nextNode, u_int64 emptySize, u_int64 lastEmpty, u_int64 nextEmpty);

        static EmptyNode *parse(std::istream &input);

        ByteArray toBytes() override;

        [[nodiscard]] std::string toString(u_int64 position = 0) const;

        u_int64 lastNode;
        u_int64 nextNode;
        u_int64 emptySize;
        u_int64 lastEmpty;
        u_int64 nextEmpty;

    };

} // FileSystem

#endif //FILESYSTEM_EMPTYNODE_H
