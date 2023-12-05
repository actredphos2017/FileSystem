//
// Created by actre on 11/23/2023.
// Encoded by UTF-8
//

#ifndef FILESYSTEM_DISKENTITY_H
#define FILESYSTEM_DISKENTITY_H


#include <cstddef>
#include <vector>
#include <fstream>
#include <functional>
#include "FileNode.h"
#include "Utils.h"
#include "EmptyNode.h"
#include "FileLinker.h"

namespace FileSystem {

    const u_int64 MAX_BYTE_SIZE = u_int64{0xFFFFFFFFFFFFFFFF}; // 8 字节

    const static u_int64 LAST_NODE_START = 4;
    const static u_int64 NEXT_NODE_START = 12;

    /**
     * 系统最大支持空间： 2^64 Byte = 2^54 KB = 2^44 MB = 2^34 GB
     * 存储格式： | 文件系统标识 8 字节 | 磁盘大小 8 字节 |  Root 根目录头文件地址 8 字节 | 空闲链表头地址 8 字节 | 超级用户密码 32 字节 | 文件数据 |
     * 文件索引开始位置 40 字节
     */


    const u_int64 UNDEFINED = 0;

    class DiskEntity {

        const static u_int64 ROOT_START = 16;
        const static u_int64 EMPTY_START = 24;
        const static u_int64 FILE_INDEX_START = 64;

    public:
        // 创建新的虚拟磁盘
        DiskEntity(u_int64 size, const std::string& path, const std::string& root_password);

        // 加载虚拟磁盘
        explicit DiskEntity(const std::string& path);

        // 获取虚拟磁盘的根目录头文件（不存在则返回 nullptr）
        u_int64 root();

        // 获取虚拟磁盘的空闲区（不存在则返回 nullptr）
        EmptyNode *emptyAt(u_int64 position);

        // 获取虚拟磁盘中某个位置的文件（不存在则返回 nullptr）
        FileNode *fileAt(u_int64 position);

        // 删除虚拟磁盘中某个位置的文件
        void removeFileAt(u_int64 position);

        // 添加新的文件（空间已满则添加失败，返回 false）
        u_int64 addFile(const INode& iNode, ByteArray byteArray);
//
//        // 更新某个位置的文件，返回新的文件位置（若改变），空间已满，更新失败返回 nullptr
//        u_int64 *update(u_int64 originLoc, const FileNode &newFile);
//
//        // 全盘优化磁盘碎片
//        void optimize();

        INode fileINodeAt(u_int64 position);

        // 格式化磁盘
        void format(u_int64 diskSize, const std::string& rootPassword);

    private:

        void checkFormat();

        u_int64 findLastEmpty(u_int64 nowNode);
        u_int64 findNextEmpty(u_int64 nowNode);

        FileLinker _fileLinker;
    };

} // FileSystem

#endif //FILESYSTEM_DISKENTITY_H
