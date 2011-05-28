cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (commands_SRCS Command.cpp    Command.h
                   CmdCustom.cpp  CmdCustom.h
                   CmdExec.cpp    CmdExec.h
                   CmdHelp.cpp    CmdHelp.h
                   CmdInfo.cpp    CmdInfo.h
                   CmdInstall.cpp CmdInstall.h
                   CmdLogo.cpp    CmdLogo.h
                   CmdShow.cpp    CmdShow.h
                   CmdTags.cpp    CmdTags.h
                   CmdTip.cpp     CmdTip.h
                   CmdVersion.cpp CmdVersion.h)

add_library (commands STATIC ${commands_SRCS})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_C_FLAGS_DEBUG "-ggdb3")
set (CMAKE_C_FLAGS_RELEASE "-O3")
