cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (task_SRCS A3.cpp A3.h
               A3t.cpp A3t.h
               Arg.cpp Arg.h
               Color.cpp Color.h
               Config.cpp Config.h
               Context.cpp Context.h
               DOM.cpp DOM.h
               Date.cpp Date.h
               Directory.cpp Directory.h
               OldDuration.cpp OldDuration.h
               E9.cpp E9.h
               File.cpp File.h
               Hooks.cpp Hooks.h
               JSON.cpp JSON.h
               LRParser.cpp LRParser.h
               Msg.cpp Msg.h
               Nibbler.cpp Nibbler.h
               Parser.cpp Parser.h
               Path.cpp Path.h
               RX.cpp RX.h
               Socket.cpp Socket.h
               TDB2.cpp TDB2.h
               Task.cpp Task.h
               Taskmod.cpp Taskmod.h
               Timer.cpp Timer.h
               Transport.cpp Transport.h
               TransportCurl.cpp TransportCurl.h
               TransportRSYNC.cpp TransportRSYNC.h
               TransportSSH.cpp TransportSSH.h
               TransportShell.cpp TransportShell.h
               Tree.cpp Tree.h
               Uri.cpp Uri.h
               ViewTask.cpp ViewTask.h
               ViewText.cpp ViewText.h
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
               util.cpp util.h
               wcwidth6.cpp)

add_library (task STATIC ${task_SRCS})
add_executable (task_executable   main.cpp)
add_executable (parser_executable bnf.cpp)                                                                                          
add_executable (args_executable   args.cpp)                                                                                         

# Yes, 'task' is included twice, otherwise linking fails on assorted OSes.
target_link_libraries (task_executable task commands columns task ${TASK_LIBRARIES})
target_link_libraries (parser_executable task commands columns task ${TASK_LIBRARIES})                      
target_link_libraries (args_executable   task commands columns task ${TASK_LIBRARIES})                      

set_property (TARGET task_executable PROPERTY OUTPUT_NAME "task")

install (TARGETS task_executable DESTINATION ${TASK_BINDIR})
                                                                                                                                    
set_property (TARGET parser_executable PROPERTY OUTPUT_NAME "parser")                                                               
set_property (TARGET args_executable   PROPERTY OUTPUT_NAME "args")                                                                 

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
