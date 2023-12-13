//
// Created by actre on 11/24/2023.
//

#include "FileLinker.h"

#include <fstream>
#include <utility>
#include <filesystem>

namespace FileSystem {

    FileLinker::FileLinker(std::string _path) : path(std::move(_path)) {}

    bool FileLinker::exist() const {
        return std::filesystem::exists(path);
    }

    bool FileLinker::create() const {
        if (exist()) std::filesystem::remove(path);
        std::string pathCpy = path;
        std::ofstream file(pathCpy, std::ios::app | std::ios::out);
        auto good = file.good();
        file.close();
        return good;
    }

    void FileLinker::resize(u_int64 newSize) const {
        std::filesystem::resize_file(path, newSize);
    }

    u_int64 FileLinker::size() const {
        return std::filesystem::file_size(path);
    }

    void
    FileLinker::doWithFileI(u_int64 position, u_int64 offset, const std::function<void(std::ifstream &)> &f) const {
        std::string pathCpy = path;
        std::ifstream file{pathCpy, std::ios::in};
        if (file.is_open()) {
            file.seekg(static_cast<std::streampos>(position + offset), std::ios::beg);
            f(file);
            file.close();
        } else {
            throw Error("FileLinker::doWithFileI", "文件打开失败");
        }
    }

    void
    FileLinker::doWithFileO(u_int64 position, u_int64 offset, const std::function<void(std::ofstream &)> &f) const {
        std::string pathCpy = path;
        std::ofstream file(pathCpy, std::ios::out | std::ios::in | std::ios::binary);

        if (file.is_open()) {
            file.seekp(static_cast<std::streampos>(position + offset), std::ios::beg);
            f(file);
            file.close();
        } else {
            throw Error("FileLinker::doWithFileO", "文件打开失败");
        }
    }

    void FileLinker::write(u_int64 position, u_int64 offset, ByteArray byteArray) const {
        doWithFileO(position, offset, [&](std::ofstream &file) {
            file.write(reinterpret_cast<const char *>(byteArray.toBytes()),
                       static_cast<std::streamsize>(byteArray.size()));
        });
    }

    std::ifstream *FileLinker::getFileInput(u_int64 position, u_int64 offset) const {
        auto *file = new std::ifstream{path, std::ios::in};
        if (file->is_open()) {
            file->seekg(static_cast<std::streampos>(position + offset), std::ios::beg);
            return file;
        }
        throw Error{"FileLinker::getFileInput", "文件打开失败"};
    }

    template<class T>
    T FileLinker::readAt(u_int64 position, u_int64 offset) {
        char *data = static_cast<char *>(malloc(sizeof(T)));
        doWithFileI(position, offset, [&](std::ifstream &file) {
            file.read(data, sizeof(T));
        });
        T *res = static_cast<T *>(malloc(sizeof(T)));
        std::memcpy(res, data, sizeof(T));
        return *res;
    }

    template int FileLinker::readAt(u_int64 position, u_int64 offset);

    template u_int64 FileLinker::readAt(u_int64 position, u_int64 offset);

    FileLinker::~FileLinker() = default;
} // FileSystem