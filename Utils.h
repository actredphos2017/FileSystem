//
// Created by actre on 11/23/2023.
//

#pragma once

#include <cstddef>
#include <cstring>
#include <iostream>
#include <vector>
#include <list>
#include <sstream>
#include <algorithm>
#include <random>
#include <functional>

typedef unsigned long long u_int64;


void clearConsole();

class ByteArray {
public:
    ByteArray() = default;

    explicit ByteArray(std::byte byte);

    ByteArray(const std::byte *bytes, size_t length);

    std::byte *data();

    u_int64 flatSize();

    ByteArray &append(const std::byte *bytes, u_int64 length);

    ByteArray &append(const ByteArray &bytes);

    ByteArray &append(std::byte byte);

    ByteArray &read(std::istream &input, u_int64 size, bool reset);

    ByteArray subByte(u_int64 from, u_int64 to);

    [[nodiscard]] u_int64 size() const;

    std::vector<std::byte>::const_iterator cbegin();
    std::vector<std::byte>::const_iterator cend();

private:

    std::vector<std::byte> _bytes{};
};

class IByteable {

public:

    virtual ByteArray toBytes() {
        return {};
    };

    template<class T>
    static ByteArray toBytes(T data) {
        return {reinterpret_cast<std::byte *>(&data), sizeof(T)};
    }

    template<class T>
    static T fromBytes(ByteArray array) {
        static_assert(std::is_trivially_copyable<T>::value, "Type T must be trivially copyable");
        T *result = reinterpret_cast<T *>(malloc(sizeof(T)));
        std::memcpy(result, array.data(), sizeof(T));
        return *result;
    }
};

std::pair<std::string, std::list<std::string>> commandTrim(const std::string &cmd);

class size_format_error : std::exception {
};

u_int64 parseSizeString(const std::string &sizeString);

std::string randomStr(int size, const std::string &valueFrom = "abcdefghijklmnopqrstuvwxyz01345679");


namespace FileSystem {
    enum NodeType {
        File, Empty, Undefined
    };

    NodeType getType(ByteArray &bytes);

    NodeType getType(std::istream &startPos);

    std::list<std::string> splitString(const std::string &input, char delimiter);

    std::string checkPath(const std::string &path);

    class Error : std::runtime_error {
    public:
        Error(const std::string &func, const std::string &reason) :
                msg("错误发生在：" + func + ", 原因：" + reason),
                std::runtime_error("错误发生在：" + func + ", 原因：" + reason) {}


        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }

    private:
        std::string msg;
    };

    std::string filledStr(std::string str, int len);

    std::list<std::string> fixPath(const std::list<std::string> &filePath);

    std::string pathStr(const std::list<std::string> &filePath, bool addSlashAtEnd = true);

}

inline void assert(bool require, const std::string &func = "assert", const std::string &reason = "断言失败") {
    if (!require) throw FileSystem::Error{func, reason};
}

void assertLazy(bool require, const std::string &func, std::function<std::string()> fun);


using std::endl;
using std::cout;
