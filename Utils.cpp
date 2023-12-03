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
