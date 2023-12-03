//
// Created by actre on 12/2/2023.
//

#ifndef FILESYSTEM_FILETERMINAL_H
#define FILESYSTEM_FILETERMINAL_H

#include "DiskEntity.h"

namespace FileSystem {

    class FileTerminal {
    public:
        explicit FileTerminal(DiskEntity &diskEntity);

        u_int64 getFileAtPath(u_int64 position, const std::string &fileName, bool isPathStart);

        u_int64 getFilePos(const std::string &filePath);

        std::list<std::pair<u_int64, INode>> getDir(const std::string &filePath);

    private:
        DiskEntity &_diskEntity;
    };

} // FileSystem

#endif //FILESYSTEM_FILETERMINAL_H
