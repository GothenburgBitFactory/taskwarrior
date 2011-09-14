cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (task_SRCS A3.cpp A3.h
               API.cpp API.h
               Arg.cpp Arg.h
               Color.cpp Color.h
               Config.cpp Config.h
               Context.cpp Context.h
               DOM.cpp DOM.h
               Date.cpp Date.h
               Directory.cpp Directory.h
               Duration.cpp Duration.h
               E9.cpp E9.h
               File.cpp File.h
               Hooks.cpp Hooks.h
               JSON.cpp JSON.h
               Nibbler.cpp Nibbler.h
               Path.cpp Path.h
               Permission.cpp Permission.h
               RX.cpp RX.h
               TDB2.cpp TDB2.h
               Task.cpp Task.h
               Taskmod.cpp Taskmod.h
               Timer.cpp Timer.h
               Transport.cpp Transport.h
               TransportCurl.cpp TransportCurl.h
               TransportRSYNC.cpp TransportRSYNC.h
               TransportSSH.cpp TransportSSH.h
               Uri.cpp Uri.h
               ViewTask.cpp ViewTask.h
               ViewText.cpp ViewText.h
               helpers.cpp
               dependency.cpp
               feedback.cpp
               i18n.h
               interactive.cpp
               legacy.cpp
               recur.cpp
               rules.cpp
               sort.cpp
               text.cpp text.h
               utf8.cpp utf8.h
               util.cpp util.h)

add_library (task STATIC ${task_SRCS})
add_executable (task_executable main.cpp)

# Yes, 'task' is included twice, other linking fails on CentOS.
target_link_libraries (task_executable task commands columns task ${TASK_LIBRARIES})

set_property (TARGET task_executable PROPERTY OUTPUT_NAME "task")

install (TARGETS task_executable DESTINATION ${TASK_BINDIR})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -ggdb3 -Wall")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
