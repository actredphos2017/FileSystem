//
// Created by actre on 11/24/2023.
//

#include "FileLinker.h"

#include <fstream>

namespace FileSystem {
    FileLinker::FileLinker(std::string_view path) {
        _fileIO = std::fstream{path.data(), std::ios::in | std::ios::out | std::ios::binary};
        if (!_fileIO) {
            throw std::exception{};
        }
    }

    std::fstream &FileLinker::getFileIO(u_int64 position, u_int64 offset) {
        _fileIO.flush();
        _fileIO.seekg((long) (position + offset), std::ios::beg);
        return _fileIO;
    }

    void FileLinker::write(u_int64 position, u_int64 offset, ByteArray byteArray) {
        _fileIO.seekg((long) (position + offset), std::ios::beg);
        _fileIO.write(reinterpret_cast<const char *>(byteArray.toBytes()), (long) byteArray.size());
    }

    void FileLinker::resize(u_int64 size) {
        _fileIO.seekp((long) (size - 1l));
        _fileIO.put((unsigned char) {0x0});
    }

    FileLinker::~FileLinker() {
        _fileIO.close();
    }
} // FileSystem