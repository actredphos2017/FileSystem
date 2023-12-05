//
// Created by actre on 11/24/2023.
//

#ifndef FILESYSTEM_FILELINKER_H
#define FILESYSTEM_FILELINKER_H

#include "Utils.h"
#include <fstream>
#include <functional>

namespace FileSystem {

    class FileLinker {
    public:
        explicit FileLinker(std::string path);

        bool exist() const;

        bool create() const;

        void resize(u_int64 size) const;

        u_int64 size() const;

        void doWithFileI(u_int64 position, u_int64 offset, const std::function<void(std::ifstream &)> &f) const;

        void doWithFileO(u_int64 position, u_int64 offset, const std::function<void(std::ofstream &)> &f) const;

        void write(u_int64 position, u_int64 offset, ByteArray byteArray) const;

        template<class T>
        T readAt(u_int64 position, u_int64 offset);

        ~FileLinker();

        // private:
        std::string path;
    };

} // FileSystem

#endif //FILESYSTEM_FILELINKER_H
