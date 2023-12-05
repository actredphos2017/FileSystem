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

        std::string getPath();

        u_int64 getFileAtPath(u_int64 position, const std::string &fileName, bool isPathStart);

        u_int64 getFilePos(std::list<std::string> _folderPath);

        u_int64 createDir(std::list<std::string> _folderPath, std::string fileName);

        std::list<std::pair<u_int64, INode>> getDir(std::list<std::string> filePath);

    private:
        DiskEntity *_diskEntity{nullptr};
    };

} // FileSystem

#endif //FILESYSTEM_FSCONTROLLER_H
