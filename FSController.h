//
// Created by actre on 12/2/2023.
//

#ifndef FILESYSTEM_FSCONTROLLER_H
#define FILESYSTEM_FSCONTROLLER_H

#include "DiskEntity.h"

namespace FileSystem {

    class FSController {

    public:
        [[nodiscard]] bool good() const;

        void create(u_int64 size, std::string path, const std::string &root_password);

        void setPath(std::string path);

        [[nodiscard]] std::string getDiskTitle() const;

        u_int64 createDir(const std::list<std::string> &_folderPath, std::string fileName);

        std::list<std::pair<u_int64, INode>> getDir(const std::list<std::string>& filePath);

        INode getINodeByPath(const std::list<std::string>& folderPath);


#ifndef FS_DEBUG
    private:
#endif

        [[nodiscard]] u_int64 getFileAtFolder(u_int64 position, const std::string &fileName, bool isRoot) const;

        [[nodiscard]] u_int64 getFilePos(const std::list<std::string>& _folderPath) const;

        DiskEntity *_diskEntity{nullptr};

    };

} // FileSystem

#endif //FILESYSTEM_FSCONTROLLER_H
