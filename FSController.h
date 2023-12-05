//
// Created by actre on 12/2/2023.
//

#ifndef FILESYSTEM_FSCONTROLLER_H
#define FILESYSTEM_FSCONTROLLER_H

#include "DiskEntity.h"

namespace FileSystem {

    class FSController {
    public:

        bool good() const;

        void create(u_int64 size, std::string path, const std::string &root_password);

        void setPath(std::string path);

        u_int64 getFileAtPath(u_int64 position, const std::string &fileName, bool isPathStart);

        u_int64 getFilePos(const std::string &filePath);

        std::list<std::pair<u_int64, INode>> getDir(const std::string &filePath);

        DiskEntity *_diskEntity{nullptr};
    };

} // FileSystem

#endif //FILESYSTEM_FSCONTROLLER_H
