//
// Created by actre on 11/23/2023.
//

#include "Utils.h"


ByteArray::ByteArray(std::byte byte) {
    this->_bytes.push_back(byte);
}

ByteArray::ByteArray(const std::byte *bytes, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        _bytes.push_back(bytes[i]);
    }
}

std::byte *ByteArray::toBytes() {
    return _bytes.data();
}

ByteArray &ByteArray::append(std::byte byte) {
    _bytes.push_back(byte);
    return *this;
}

ByteArray &ByteArray::append(const ByteArray &bytes) {
    this->_bytes.insert(_bytes.end(), bytes._bytes.begin(), bytes._bytes.end());
    return *this;
}

ByteArray &ByteArray::append(const std::byte *bytes, u_int64 length) {
    for (u_int64 i = 0; i < length; ++i) {
        _bytes.push_back(bytes[i]);
    }
    return *this;
}

u_int64 ByteArray::size() const {
    return this->_bytes.size();
}

ByteArray &ByteArray::read(std::istream &input, u_int64 size, bool reset) {
    std::streampos originPos;
    if (reset) originPos = input.tellg();
    char buf{};
    for (u_int64 i = 0; i < size; ++i) {
        input.get(buf);
        this->append(reinterpret_cast<const std::byte *>(&buf), 1);
    }
    if (reset) input.seekg(originPos);
    return *this;
}

ByteArray ByteArray::subByte(u_int64 from, u_int64 to) {
    auto res = ByteArray();

    for (u_int64 i = from; i < to; ++i) {
        res.append(this->_bytes[i]);
    }

    return res;
}

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

    std::string checkPath(const std::string &path) {
        std::string trimmedPath = path;
        size_t start = trimmedPath.find_first_not_of(" \n\r\t");
        size_t end = trimmedPath.find_last_not_of(" \n\r\t");

        if (start != std::string::npos && end != std::string::npos) {
            trimmedPath = trimmedPath.substr(start, end - start + 1);
        } else {
            return "";
        }

        return trimmedPath;
    }
}

