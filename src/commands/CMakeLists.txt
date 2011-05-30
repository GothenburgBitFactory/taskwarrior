cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (commands_SRCS Command.cpp        Command.h
                   CmdAdd.cpp         CmdAdd.h
                   CmdAnnotate.cpp    CmdAnnotate.h
                   CmdAppend.cpp      CmdAppend.h
                   CmdBurndown.cpp    CmdBurndown.h
                   CmdCommands.cpp    CmdCommands.h
                   CmdColor.cpp       CmdColor.h
                   CmdConfig.cpp      CmdConfig.h
                   CmdCount.cpp       CmdCount.h
                   CmdCustom.cpp      CmdCustom.h
                   CmdDenotate.cpp    CmdDenotate.h
                   CmdDiagnostics.cpp CmdDiagnostics.h
                   CmdDuplicate.cpp   CmdDuplicate.h
                   CmdEdit.cpp        CmdEdit.h
                   CmdExec.cpp        CmdExec.h
                   CmdHelp.cpp        CmdHelp.h
                   CmdHistory.cpp     CmdHistory.h
                   CmdIDs.cpp         CmdIDs.h
                   CmdImport.cpp      CmdImport.h
                   CmdInfo.cpp        CmdInfo.h
                   CmdInstall.cpp     CmdInstall.h
                   CmdLog.cpp         CmdLog.h
                   CmdLogo.cpp        CmdLogo.h
                   CmdPrepend.cpp     CmdPrepend.h
                   CmdProjects.cpp    CmdProjects.h
                   CmdQuery.cpp       CmdQuery.h
                   CmdReports.cpp     CmdReports.h
                   CmdShell.cpp       CmdShell.h
                   CmdShow.cpp        CmdShow.h
                   CmdStart.cpp       CmdStart.h
                   CmdStatistics.cpp  CmdStatistics.h
                   CmdStop.cpp        CmdStop.h
                   CmdSummary.cpp     CmdSummary.h
                   CmdTags.cpp        CmdTags.h
                   CmdTip.cpp         CmdTip.h
                   CmdUrgency.cpp     CmdUrgency.h
                   CmdVersion.cpp     CmdVersion.h)

add_library (commands STATIC ${commands_SRCS})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -ggdb3 -Wall")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")
