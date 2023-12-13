//
// Created by actre on 11/24/2023.
//

#include "EmptyNode.h"

namespace FileSystem {


    EmptyNode::EmptyNode(u_int64 lastNode, u_int64 nextNode, u_int64 emptySize, u_int64 lastEmpty, u_int64 nextEmpty) :
            lastNode(lastNode),
            nextNode(nextNode),
            emptySize(emptySize),
            lastEmpty(lastEmpty),
            nextEmpty(nextEmpty) {}

    EmptyNode *EmptyNode::parse(std::istream &input) {
        ByteArray().read(input, 4, false);

        auto _1 = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto _2 = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto _3 = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto _4 = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));
        auto _5 = IByteable::fromBytes<u_int64>(ByteArray().read(input, 8, false));

        return new EmptyNode(_1, _2, _3, _4, _5);
    }

    ByteArray EmptyNode::toBytes() {
        return ByteArray()
                .append(reinterpret_cast<const std::byte *>("EMPT"), 4)
                .append(IByteable::toBytes(lastNode))
                .append(IByteable::toBytes(nextNode))
                .append(IByteable::toBytes(emptySize))
                .append(IByteable::toBytes(lastEmpty))
                .append(IByteable::toBytes(nextEmpty));
    }

    std::string EmptyNode::toString(u_int64 pos) const {
        std::stringstream ss;

        ss << "===== EMPTY =====" << std::hex << endl;

        if (pos != 0) {
            ss << "DiskPos: "<< pos << endl;
        }
        ss << "LastNode: " << lastNode << endl;
        ss << "NextNode: " << nextNode << endl;
        ss << "EmptySize: " << emptySize << endl;
        ss << "LastEmpty: " << lastEmpty << endl;
        ss << "NextEmpty: " << nextEmpty << endl;

        ss << std::dec;

        return ss.str();
    }


} // FileSystem