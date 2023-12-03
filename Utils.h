//
// Created by actre on 11/23/2023.
//

#ifndef FILESYSTEM_UTILS_H
#define FILESYSTEM_UTILS_H

#include <cstddef>
#include <cstring>
#include <iostream>
#include <vector>
#include <list>
#include <sstream>

typedef unsigned long long u_int64;


inline void assert(bool require);

class ByteArray {
public:
    ByteArray() = default;

    explicit ByteArray(std::byte byte);

    ByteArray(const std::byte *bytes, size_t length);

    std::byte *toBytes();

    ByteArray &append(const std::byte *bytes, u_int64 length);

    ByteArray &append(const ByteArray &bytes);

    ByteArray &append(std::byte byte);

    ByteArray &read(std::istream &input, u_int64 size, bool reset);

    ByteArray subByte(u_int64 from, u_int64 to);

    [[nodiscard]] u_int64 size() const;

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
        T result;
        std::memcpy(&result, array.toBytes(), sizeof(T));
        return result;
    }
};

std::pair<std::string, std::list<std::string>> commandTrim(const std::string &cmd) {
    std::string trimmedCmd = cmd;
    size_t start = trimmedCmd.find_first_not_of(" \n\r\t");
    size_t end = trimmedCmd.find_last_not_of(" \n\r\t");

    if (start != std::string::npos && end != std::string::npos) {
        trimmedCmd = trimmedCmd.substr(start, end - start + 1);
    } else {
        return {"", {}};
    }

    std::istringstream iss(trimmedCmd);
    std::string command;
    iss >> command;

    std::list<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    return {command, args};
}

class size_format_error : std::exception {
};

u_int64 parseSizeString(const std::string &sizeString) {
    std::size_t pos = 0;

    u_int64 size = std::stoull(sizeString, &pos);

    std::string unit = sizeString.substr(pos);

    if (unit == "B") {
        size *= 1;
    } else if (unit == "KB" || unit == "K") {
        size *= 1024;
    } else if (unit == "MB" || unit == "M") {
        size *= 1024 * 1024;
    } else if (unit == "GB" || unit == "G") {
        size *= 1024 * 1024 * 1024;
    } else if (unit == "TB" || unit == "T") {
        size *= 1024ULL * 1024 * 1024 * 1024;
    } else if (unit == "PB" || unit == "P") {
        size *= 1024ULL * 1024 * 1024 * 1024 * 1024;
    } else {
        throw size_format_error{};
    }

    return size;
}

namespace FileSystem {
    enum NodeType {
        File, Empty, Undefined
    };

    NodeType getType(std::istream &startPos) {
        char *identification;
        startPos.read(identification, 4);
        std::string str{identification, 4};
        if (str == "FILE") return FileSystem::File;
        if (str == "EMPT") return FileSystem::Empty;
        return FileSystem::Undefined;
    }

    std::list<std::string> splitString(const std::string &input, char delimiter) {
        std::list<std::string> result;
        std::istringstream stream{input};
        std::string token;

        while (std::getline(stream, token, delimiter)) {
            result.push_back(token);
        }

        return result;
    }

    const std::string &checkPath(const std::string &path) {
        throw std::exception{};
    }
}

#endif