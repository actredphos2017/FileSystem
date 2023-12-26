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
#include "SHA256.h"
#include "EmptyNode.h"
#include "FileLinker.h"

namespace FileSystem {

    const u_int64 MAX_BYTE_SIZE = u_int64{0xFFFFFFFFFFFFFFFF}; // 8 字节

    const static u_int64 LAST_NODE_START = 4;
    const static u_int64 NEXT_NODE_START = 12;

    /**
     * 系统最大支持空间： 2^64 Byte = 2^54 KB = 2^44 MB = 2^34 GB
     * 存储格式： | 文件系统标识 8 字节 | 磁盘大小 8 字节 |  Root 根目录头文件地址 8 字节 | 空闲链表头地址 8 字节 | 超级用户密码 32 字节 | 文件数据 |
     * 文件索引开始位置 64 字节
     */

    typedef struct {
        NodeType type;
        u_int64 position;
        union {
            FileNode *file;
            EmptyNode *empty;
        } ptr;
    } NodePtr;


    const u_int64 UNDEFINED = 0;

    class DiskEntity {

        const static u_int64 DISK_SIZE_START = 8;
        const static u_int64 ROOT_START = 16;
        const static u_int64 EMPTY_START = 24;
        const static u_int64 FILE_INDEX_START = 64;
        const static u_int64 SUPERUSER_PASSWORD_START = 32;

    public:
        DiskEntity(u_int64 size, std::string path, const std::string &root_password);

        explicit DiskEntity(std::string path);

        u_int64 root();

        void setRoot(u_int64 pos);

        EmptyNode *emptyAt(u_int64 position);

        FileNode *fileAt(u_int64 position);

        NodePtr nodeAt(u_int64 position);

        void removeFileAt(u_int64 position);

        u_int64 addFile(const INode &iNode, ByteArray byteArray);

        void updateWithoutSizeChange(u_int64 originLoc, FileNode &newFile);

        void updateNextAt(u_int64 originLoc, u_int64 newNext);

        void updateFirstEmpty(u_int64 firstEmpty);

        u_int64 getFirstEmpty();

        bool assertSuperUser(std::string password);

        std::list<NodePtr> getAll();

        INode fileINodeAt(u_int64 position);

        void format(u_int64 diskSize, const std::string &rootPassword);

        void format(const std::string &rootPassword);

        std::string getPath() const;

    private:

        void checkFormat();

        u_int64 findLastEmpty(u_int64 nowNode);

        u_int64 findNextEmpty(u_int64 nowNode);

        FileLinker _fileLinker;

    };

} // FileSystem

#endif //FILESYSTEM_DISKENTITY_H
