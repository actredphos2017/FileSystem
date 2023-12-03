//
// Created by actre on 11/23/2023.
//

#ifndef FILESYSTEM_PATHDATA_H
#define FILESYSTEM_PATHDATA_H

#include <utility>

#include "Utils.h"

namespace FileSystem {

    /**
     * 结构： | 子目录链表头文件 8 字节 |
     */

    struct PathData : IByteable {

        explicit PathData(u_int64 subItemHead) :
                subItemHead(subItemHead) {}

        u_int64 subItemHead;

        ByteArray toBytes() override {
            return IByteable::toBytes(subItemHead);
        }
    };

} // FileSystem

#endif //FILESYSTEM_PATHDATA_H
