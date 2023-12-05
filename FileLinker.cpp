//
// Created by actre on 11/24/2023.
//

#include "FileLinker.h"

#include <fstream>
#include <utility>

namespace FileSystem {


    FileLinker::FileLinker(std::string path) : path(std::move(path)) {}

    bool FileLinker::exist() {
        std::ifstream file(path);
        return file.good();
    }

    bool FileLinker::createIfNotExist() {
        if (!exist()) {
            std::ofstream file(path, std::ios::binary);
            return file.good();
        }
        return true;
    }

    bool FileLinker::createCompulsory() {
        std::ofstream file(path, std::ios::binary);
        return file.good();
    }

    void FileLinker::resize(u_int64 newSize) {
        std::ofstream file(path, std::ios::binary | std::ios::ate);
        file.seekp(static_cast<std::streampos>(newSize - 1));
        file.put('\0');
    }

    u_int64 FileLinker::size() {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        return static_cast<u_int64>(file.tellg());
    }

    void FileLinker::doWithFileI(u_int64 position, u_int64 offset, const std::function<void(std::ifstream &)> &f) {
        std::ifstream file(path, std::ios::binary);
        if (file) {
            file.seekg(static_cast<std::streampos>(position + offset), std::ios::beg);
            f(file);
            file.close();
        } else {
            std::cerr << "Error opening file for input." << std::endl;
        }
    }

    void FileLinker::doWithFileO(u_int64 position, u_int64 offset, const std::function<void(std::ofstream &)> &f) {
        std::ofstream file(path, std::ios::binary);
        if (file) {
            file.seekp(static_cast<std::streampos>(position + offset), std::ios::beg);
            f(file);
            file.close();
        } else {
            std::cerr << "Error opening file for output." << std::endl;
        }
    }

    void FileLinker::write(u_int64 position, u_int64 offset, ByteArray byteArray) {
        doWithFileO(position, offset, [&](std::ofstream &file) {
            file.write(reinterpret_cast<const char *>(byteArray.toBytes()),
                       static_cast<std::streamsize>(byteArray.size()));
        });
    }

    FileLinker::~FileLinker() = default;
} // FileSystem