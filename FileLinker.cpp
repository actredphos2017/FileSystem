//
// Created by actre on 11/24/2023.
//

#include "FileLinker.h"

#include <fstream>

namespace FileSystem {
    FileLinker::FileLinker(const std::string &path) {
        _fileIO = std::fstream{};
        _fileIO.open(path, std::ios::in | std::ios::out);

        if (!_fileIO.is_open()) {
            _fileIO.close();
            std::ofstream fileCreate{path};
            assert(fileCreate.is_open(), "FileLinker::FileLinker", "文件创建失败！");
            fileCreate.close();
        } else return;
        _fileIO.open(path, std::ios::in | std::ios::out);
        assert(_fileIO.is_open(), "FileLinker::FileLinker", "文件打开失败！");
    }

    std::fstream &FileLinker::getFileIO(u_int64 position, u_int64 offset) {
        assert(_fileIO.is_open(), "FileLinker::getFileIO", "未预料到的文件关闭！");
        auto pos = (long) (position + offset);
        _fileIO.flush();
        _fileIO.seekg(pos, std::ios::beg);

        _fileIO.clear();
        _fileIO.seekp(pos, std::ios::beg);
        return _fileIO;
    }

    void FileLinker::write(u_int64 position, u_int64 offset, ByteArray byteArray) {
        assert(_fileIO.is_open(), "FileLinker::write", "未预料到的文件关闭！");
        _fileIO.seekp((long) (position + offset), std::ios::beg);
        _fileIO.write(reinterpret_cast<const char *>(byteArray.toBytes()), (long) byteArray.size());
    }

    void FileLinker::resize(u_int64 size) {
        assert(_fileIO.is_open(), "FileLinker::resize", "未预料到的文件关闭！");
        _fileIO.seekp((long) (size - 1l));
        _fileIO.put((unsigned char) {0x0});
    }

    FileLinker::~FileLinker() {
        _fileIO.close();
    }

    u_int64 FileLinker::size() {
        assert(_fileIO.is_open(), "FileLinker::size", "未预料到的文件关闭！");
        _fileIO.seekg(0, std::ios::end);
        return _fileIO.tellg();
    }
} // FileSystem