cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${TASK_INCLUDE_DIRS})

add_library (task CLI2.cpp CLI2.h
                  Color.cpp Color.h
                  Config.cpp Config.h
                  Context.cpp Context.h
                  DOM.cpp DOM.h
                  Dates.cpp Dates.h
                  Eval.cpp Eval.h
                  Filter.cpp Filter.h
                  FS.cpp FS.h
                  Hooks.cpp Hooks.h
                  ISO8601.cpp ISO8601.h
                  Lexer.cpp Lexer.h
                  Msg.cpp Msg.h
                  Nibbler.cpp Nibbler.h
                  TDB2.cpp TDB2.h
                  Task.cpp Task.h
                  Timer.cpp Timer.h
                  TLSClient.cpp TLSClient.h
                  Variant.cpp Variant.h
                  ViewTask.cpp ViewTask.h
                  ViewText.cpp ViewText.h
                  dependency.cpp
                  feedback.cpp
                  i18n.h
                  legacy.cpp
                  recur.cpp
                  rules.cpp
                  sort.cpp
                  text.cpp text.h
                  util.cpp util.h)

add_library (libshared libshared/src/JSON.cpp          libshared/src/JSON.h
                       libshared/src/Pig.cpp           libshared/src/Pig.h
                       libshared/src/RX.cpp            libshared/src/RX.h
                       libshared/src/unicode.cpp       libshared/src/unicode.h
                       libshared/src/utf8.cpp          libshared/src/utf8.h
                       libshared/src/wcwidth6.cpp)

add_executable (task_executable main.cpp)
add_executable (calc_executable calc.cpp)
add_executable (lex_executable lex.cpp)

# Yes, 'task' is included twice, otherwise linking fails on assorted OSes.
target_link_libraries (task_executable task commands columns libshared task ${TASK_LIBRARIES})
target_link_libraries (calc_executable task commands columns libshared task ${TASK_LIBRARIES})
target_link_libraries (lex_executable  task commands columns libshared task ${TASK_LIBRARIES})

set_property (TARGET task_executable PROPERTY OUTPUT_NAME "task")

install (TARGETS task_executable DESTINATION ${TASK_BINDIR})

set_property (TARGET calc_executable PROPERTY OUTPUT_NAME "calc")
set_property (TARGET lex_executable PROPERTY OUTPUT_NAME "lex")

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
