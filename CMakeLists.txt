cmake_minimum_required(VERSION 3.22)
project(FileSystem)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        FileSystem
        Utils.h
        Utils.cpp
        PathData.h
        PathData.cpp
        DiskEntity.h
        DiskEntity.cpp
        FileNode.h
        FileNode.cpp
        main.cpp
        EmptyNode.h
        EmptyNode.cpp
        FileLinker.h
        FileLinker.cpp
        FileTerminal.cpp
        FileTerminal.h
        CommandService.cpp
        CommandService.h

)
