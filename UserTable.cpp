//
// Created by actre on 12/23/2023.
//

#include "UserTable.h"

#include <utility>

#include "SHA256.h"

FileSystem::UserItem::UserItem(std::string username, const std::string &password) :
        username(std::move(username)),
        pwdSha(Ly::Sha256::getInstance().getHexMessageDigest(password)) {}

bool FileSystem::UserTable::pushUser(FileSystem::UserItem userItem) {

    if (std::any_of(users.begin(), users.end(),
                    [&](const UserItem &a) -> bool { return a.username == userItem.username; }))
        return false;

    users.push_back(userItem);

    return true;
}

ByteArray FileSystem::UserTable::toBytes() {

    auto res = ByteArray();
    for (const auto& user: users) {
        res.append(reinterpret_cast<const std::byte *> (user.username.c_str()), user.username.size());
        res.append(std::byte{'\0'});
        res.append(reinterpret_cast<const std::byte *>(user.pwdSha.c_str()), user.pwdSha.size());
        res.append(std::byte{'\0'});
    }

    return res;
}

FileSystem::UserTable *FileSystem::UserTable::parse(ByteArray bytes) {
    auto userTable = new UserTable();

    auto it = bytes.cbegin();
    while (it != bytes.cend()) {
        std::string username;
        while (*it != std::byte{'\0'}) {
            username += static_cast<const char>(*it);
            ++it;
        }
        ++it;

        std::string pwdSha;
        while (*it != std::byte{'\0'}) {
            pwdSha += static_cast<const char>(*it);
            ++it;
        }
        ++it;

        UserItem user;
        user.username = username;
        user.pwdSha = pwdSha;
        userTable->pushUser(user);
    }

    return userTable;
}

FileSystem::UserItem *FileSystem::UserTable::login(std::string username, std::string password) {
    UserItem* res = nullptr;

    for (const auto& user : users) {
        if (user.username == username) {
            res = new UserItem{user};
            break;
        }
    }

    if (res == nullptr) return nullptr;

    if (Ly::Sha256::getInstance().getHexMessageDigest(password) == res->pwdSha)
        return res;

    return nullptr;
}
