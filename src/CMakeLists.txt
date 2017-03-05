cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${TASK_INCLUDE_DIRS})

add_library (task CLI2.cpp CLI2.h
                  Context.cpp Context.h
                  DOM.cpp DOM.h
                  Dates.cpp Dates.h
                  Eval.cpp Eval.h
                  Filter.cpp Filter.h
                  Hooks.cpp Hooks.h
                  Lexer.cpp Lexer.h
                  Nibbler.cpp Nibbler.h
                  TDB2.cpp TDB2.h
                  Task.cpp Task.h
                  TLSClient.cpp TLSClient.h
                  Variant.cpp Variant.h
                  ViewTask.cpp ViewTask.h
                  dependency.cpp
                  feedback.cpp
                  i18n.h
                  legacy.cpp
                  recur.cpp
                  rules.cpp
                  sort.cpp
                  util.cpp util.h)

add_library (libshared libshared/src/Color.cpp         libshared/src/Color.h
                       libshared/src/Configuration.cpp libshared/src/Configuration.h
                       libshared/src/Datetime.cpp      libshared/src/Datetime.h
                       libshared/src/Duration.cpp      libshared/src/Duration.h
                       libshared/src/FS.cpp            libshared/src/FS.h
                       libshared/src/JSON.cpp          libshared/src/JSON.h
                       libshared/src/Msg.cpp           libshared/src/Msg.h
                       libshared/src/Pig.cpp           libshared/src/Pig.h
                       libshared/src/RX.cpp            libshared/src/RX.h
                       libshared/src/Table.cpp         libshared/src/Table.h
                       libshared/src/Timer.cpp         libshared/src/Timer.h
                       libshared/src/format.cpp        libshared/src/format.h
                       libshared/src/ip.cpp
                       libshared/src/shared.cpp        libshared/src/shared.h
                       libshared/src/unicode.cpp       libshared/src/unicode.h
                       libshared/src/utf8.cpp          libshared/src/utf8.h
                       libshared/src/wcwidth6.cpp)

add_executable (task_executable main.cpp)
add_executable (calc_executable calc.cpp)
add_executable (lex_executable lex.cpp)

# Yes, 'task' (and hence libshared) is included twice, otherwise linking fails on assorted OSes.
target_link_libraries (task_executable task commands columns libshared task libshared ${TASK_LIBRARIES})
target_link_libraries (calc_executable task commands columns libshared task libshared ${TASK_LIBRARIES})
target_link_libraries (lex_executable  task commands columns libshared task libshared ${TASK_LIBRARIES})

set_property (TARGET task_executable PROPERTY OUTPUT_NAME "task")

install (TARGETS task_executable DESTINATION ${TASK_BINDIR})

set_property (TARGET calc_executable PROPERTY OUTPUT_NAME "calc")
set_property (TARGET lex_executable PROPERTY OUTPUT_NAME "lex")

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
