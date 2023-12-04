cmake_minimum_required(VERSION 3.22)
project(FileSystem)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        FileSystem
        Utils.h
        Utils.cpp
        DiskEntity.h
        DiskEntity.cpp
        FileNode.h
        FileNode.cpp
        EmptyNode.h
        EmptyNode.cpp
        FileLinker.h
        FileLinker.cpp
        FileSystemConnector.cpp
        FileSystemConnector.h
        Terminal.cpp
        Terminal.h
        SHA256.cpp
        SHA256.h
        main.cpp
)