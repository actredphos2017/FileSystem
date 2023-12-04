//
// Created by actre on 11/24/2023.
//

#ifndef FILESYSTEM_FILELINKER_H
#define FILESYSTEM_FILELINKER_H

#include "Utils.h"
#include <fstream>

namespace FileSystem {

    class FileLinker {
    public:
        explicit FileLinker(const std::string &path);

        void resize(u_int64 size);

        u_int64 size();

        std::fstream &getFileIO(u_int64 position = 0, u_int64 offset = 0);

        void write(u_int64 position, u_int64 offset, ByteArray byteArray);

        template<class T>
        T readAt(u_int64 position, u_int64 offset) {
            return IByteable::fromBytes<T>(ByteArray().read(getFileIO(position + offset), sizeof(T), true));
        }

        ~FileLinker();

    private:
        std::fstream _fileIO;
    };

} // FileSystem

#endif //FILESYSTEM_FILELINKER_H
