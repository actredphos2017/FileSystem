//
// Created by actre on 12/23/2023.
//

#ifndef FILESYSTEM_USERTABLE_H
#define FILESYSTEM_USERTABLE_H


#include "Utils.h"

namespace FileSystem {


    class UserItem {
    public:

        UserItem() = default;

        UserItem(std::string username, const std::string &password);

        std::string username{};
        std::string pwdSha{};

    };

    class UserTable : IByteable {

    public:

        static UserTable *parse(ByteArray bytes);

        ByteArray toBytes() override;

        bool pushUser(UserItem userItem);

        UserItem *login(std::string username, std::string password);

    private:

        std::vector<UserItem> users{};

    };
}

#endif //FILESYSTEM_USERTABLE_H
