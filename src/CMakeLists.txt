cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (task_SRCS API.cpp API.h
               Arguments.cpp Arguments.h
               Att.cpp Att.h
               Cmd.cpp Cmd.h
               Color.cpp Color.h
               Config.cpp Config.h
               Context.cpp Context.h
               DOM.cpp DOM.h
               Date.cpp Date.h
               Directory.cpp Directory.h
               Duration.cpp Duration.h
               File.cpp File.h
               Filter.cpp Filter.h
               Hooks.cpp Hooks.h
               JSON.cpp JSON.h
               Location.cpp Location.h
               Nibbler.cpp Nibbler.h
               Path.cpp Path.h
               Permission.cpp Permission.h
               Record.cpp Record.h
               Sequence.cpp Sequence.h
               Subst.cpp Subst.h
               TDB.cpp TDB.h
               TDB2.cpp TDB2.h
               Task.cpp Task.h
               Taskmod.cpp Taskmod.h
               Timer.cpp Timer.h
               Transport.cpp Transport.h
               TransportCurl.cpp TransportCurl.h
               TransportRSYNC.cpp TransportRSYNC.h
               TransportSSH.cpp TransportSSH.h
               Uri.cpp Uri.h
               Variant.cpp Variant.h
               ViewTask.cpp ViewTask.h
               ViewText.cpp ViewText.h
               burndown.cpp
               command.cpp
               dependency.cpp
               export.cpp
               feedback.cpp
               i18n.h
               import.cpp
               interactive.cpp
               recur.cpp
               report.cpp
               rules.cpp
               rx.cpp rx.h
               sort.cpp
               text.cpp text.h
               utf8.cpp utf8.h
               util.cpp util.h)

add_library (task STATIC ${task_SRCS})
add_executable (task_executable main.cpp)
target_link_libraries (task_executable task commands columns ${TASK_LIBRARIES})
set_property (TARGET task_executable PROPERTY OUTPUT_NAME "task")

install (TARGETS task_executable DESTINATION ${TASK_BINDIR})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_C_FLAGS_DEBUG "-ggdb3")
set (CMAKE_C_FLAGS_RELEASE "-O3")
