//
// Created by actre on 11/23/2023.
//

#include "Utils.h"


#ifdef _WIN32

void clearConsole() {
    system("cls");
}

#elif defined(__unix__)

void clearConsole() {
    system("clear");
}

#elif defined(__APPLE__)

#include <cstdlib>

void clearConsole() {
    system("clear");
}

#endif

ByteArray::ByteArray(std::byte byte) {
    this->_bytes.push_back(byte);
}

ByteArray::ByteArray(const std::byte *bytes, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        _bytes.push_back(bytes[i]);
    }
}

std::byte *ByteArray::data() {
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
    u_int64 originPos = input.tellg();
    u_int64 endPos = originPos + size;
    auto buf = char{'\0'};
    while (input.tellg() != endPos) {
        input.get(buf);
        this->append(static_cast<std::byte>(buf));
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

u_int64 ByteArray::flatSize() {
    std::vector<std::byte> bytes{_bytes};
    if (bytes.empty()) return 0;
    while (static_cast<char>(bytes.back()) == '\0') {
        bytes.pop_back();
    }
    return bytes.size();
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

std::string randomStr(int size, const std::string &valueFrom) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, valueFrom.size() - 1);

    std::string result;
    for (int i = 0; i < size; ++i) {
        result += valueFrom[distrib(gen)];
    }

    return result;
}

void assertLazy(bool require, const std::string &func, std::function<std::string()> fun) {
    if (!require) throw FileSystem::Error{func, fun()};
}

namespace FileSystem {
    NodeType getType(std::istream &startPos) {
        char *identification = static_cast<char *>(malloc(4));
        startPos.read(identification, 4);
        std::string str{identification, 4};
        if (str == "FILE") return FileSystem::File;
        if (str == "EMPT") return FileSystem::Empty;
        return FileSystem::Undefined;
    }

    NodeType getType(ByteArray &bytes) {

        assert(bytes.size() >= 4);

        std::string str{reinterpret_cast<char *>(bytes.data()), 4};
        if (str == "FILE") return FileSystem::File;
        if (str == "EMPT") return FileSystem::Empty;
        return FileSystem::Undefined;
    }

    std::list<std::string> splitString(const std::string &input, char delimiter) {
        std::list<std::string> result;
        std::string token;

        for (char ch: input) {
            if (ch != delimiter) {
                token += ch;
            } else {
                result.push_back(token);
                token.clear();
            }
        }

        if (!token.empty()) {
            result.push_back(token);
        }

        if (result.empty()) {
            result.push_back("");
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

    std::list<std::string> fixPath(const std::list<std::string> &filePath) {
        std::list<std::string> res{};
        for (const std::string &part: filePath) {
            assert(!part.empty(), "Utils::fixPath", "路径非法");
            if (part == ".") continue;
            if (part == "..") {
                assert(!res.empty(), "Utils::fixPath", "路径非法");
                res.pop_back();
            } else res.push_back(part);
        }
        return res;
    }

    std::string filledStr(std::string str, int len) {
        while (str.size() < len)
            str += ' ';
        return str;
    }

    std::string pathStr(const std::list<std::string> &filePath, bool addSlashAtEnd) {
        std::stringstream ss;
        for (const auto &it: filePath)
            ss << "/" << it;
        if (addSlashAtEnd) ss << "/";
        return ss.str();
    }
}