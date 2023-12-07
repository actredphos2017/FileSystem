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

        std::list<INode> getDir(const std::list<std::string> &filePath);

        INode getINodeByPath(const std::list<std::string> &folderPath);


#ifndef FS_DEBUG
    private:
#endif


        /**
         *
         * @param _filePath
         * @return (值, 值为 UNDEFINED 时的原因)
         * 1 尝试获取的位置为 ROOT
         *
         */
        u_int64 getFilePos(const std::list<std::string> &_filePath) const;

        DiskEntity *_diskEntity{nullptr};

    };

} // FileSystem

#endif //FILESYSTEM_FSCONTROLLER_H
