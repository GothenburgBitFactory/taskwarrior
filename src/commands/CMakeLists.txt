cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (commands_SRCS Command.cpp        Command.h
                   CmdAdd.cpp         CmdAdd.h
                   CmdAliases.cpp     CmdAliases.h
                   CmdAnnotate.cpp    CmdAnnotate.h
                   CmdAppend.cpp      CmdAppend.h
                   CmdAttributes.cpp  CmdAttributes.h
                   CmdBurndown.cpp    CmdBurndown.h
                   CmdCalc.cpp        CmdCalc.h
                   CmdCalendar.cpp    CmdCalendar.h
                   CmdCommands.cpp    CmdCommands.h
                   CmdColor.cpp       CmdColor.h
                   CmdColumns.cpp     CmdColumns.h
                   CmdConfig.cpp      CmdConfig.h
                   CmdContext.cpp     CmdContext.h
                   CmdCount.cpp       CmdCount.h
                   CmdCustom.cpp      CmdCustom.h
                   CmdDelete.cpp      CmdDelete.h
                   CmdDenotate.cpp    CmdDenotate.h
                   CmdDiagnostics.cpp CmdDiagnostics.h
                   CmdDone.cpp        CmdDone.h
                   CmdDuplicate.cpp   CmdDuplicate.h
                   CmdEdit.cpp        CmdEdit.h
                   CmdExec.cpp        CmdExec.h
                   CmdExport.cpp      CmdExport.h
                   CmdGet.cpp         CmdGet.h
                   CmdHelp.cpp        CmdHelp.h
                   CmdHistory.cpp     CmdHistory.h
                   CmdIDs.cpp         CmdIDs.h
                   CmdImport.cpp      CmdImport.h
                   CmdInfo.cpp        CmdInfo.h
                   CmdLog.cpp         CmdLog.h
                   CmdLogo.cpp        CmdLogo.h
                   CmdModify.cpp      CmdModify.h
                   CmdPrepend.cpp     CmdPrepend.h
                   CmdProjects.cpp    CmdProjects.h
                   CmdReports.cpp     CmdReports.h
                   CmdShow.cpp        CmdShow.h
                   CmdStart.cpp       CmdStart.h
                   CmdStats.cpp       CmdStats.h
                   CmdStop.cpp        CmdStop.h
                   CmdSummary.cpp     CmdSummary.h
                   CmdSync.cpp        CmdSync.h
                   CmdTags.cpp        CmdTags.h
                   CmdTimesheet.cpp   CmdTimesheet.h
                   CmdUDAs.cpp        CmdUDAs.h
                   CmdUndo.cpp        CmdUndo.h
                   CmdUnique.cpp      CmdUnique.h
                   CmdUrgency.cpp     CmdUrgency.h
                   CmdVersion.cpp     CmdVersion.h)

add_library (commands STATIC ${commands_SRCS})

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
