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

        bool exist();

        bool createIfNotExist();

        bool createCompulsory();

        void resize(u_int64 size);

        u_int64 size();

        void doWithFileI(u_int64 position, u_int64 offset, const std::function<void(std::ifstream &)> &f);

        void doWithFileO(u_int64 position, u_int64 offset, const std::function<void(std::ofstream &)> &f);

        void write(u_int64 position, u_int64 offset, ByteArray byteArray);

        template<class T>
        T readAt(u_int64 position, u_int64 offset) {
            T data;
            doWithFileI(position, offset, [&](std::ifstream &file) {
                file.read(reinterpret_cast<char *>(&data), sizeof(T));
            });
            return data;
        }

        ~FileLinker();

    private:
        std::string path;
    };

} // FileSystem

#endif //FILESYSTEM_FILELINKER_H
