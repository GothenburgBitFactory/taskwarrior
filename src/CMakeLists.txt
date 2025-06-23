cmake_minimum_required (VERSION 3.22...4.0)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${TASK_INCLUDE_DIRS})

add_library (task STATIC CLI2.cpp
                  Context.cpp
                  DOM.cpp
                  Eval.cpp
                  Filter.cpp
                  Hooks.cpp
                  Lexer.cpp
                  Operation.cpp
                  TF2.cpp
                  TDB2.cpp
                  Task.cpp
                  Variant.cpp
                  Version.cpp
                  ViewTask.cpp
                  dependency.cpp
                  feedback.cpp
                  legacy.cpp
                  nag.cpp
                  recur.cpp
                  rules.cpp
                  sort.cpp
                  util.cpp)
target_link_libraries(task taskchampion-cpp)

add_library (libshared STATIC libshared/src/Color.cpp
                       libshared/src/Configuration.cpp
                       libshared/src/Datetime.cpp
                       libshared/src/Duration.cpp
                       libshared/src/FS.cpp
                       libshared/src/JSON.cpp
                       libshared/src/Msg.cpp
                       libshared/src/Pig.cpp
                       libshared/src/RX.cpp
                       libshared/src/Table.cpp
                       libshared/src/Timer.cpp
                       libshared/src/format.cpp
                       libshared/src/ip.cpp
                       libshared/src/shared.cpp
                       libshared/src/unicode.cpp
                       libshared/src/utf8.cpp)

add_executable (task_executable main.cpp)
add_executable (calc_executable calc.cpp)
add_executable (lex_executable lex.cpp)

# Yes, 'task' (and hence libshared) is included twice, otherwise linking fails on assorted OSes.
target_link_libraries (task_executable task commands columns libshared task libshared ${TASK_LIBRARIES})
target_link_libraries (calc_executable task commands columns libshared task libshared ${TASK_LIBRARIES})
target_link_libraries (lex_executable  task commands columns libshared task libshared ${TASK_LIBRARIES})
if (DARWIN)
    # SystemConfiguration is required by Rust libraries like reqwest, to get proxy configuration.
    target_link_libraries (task_executable "-framework CoreFoundation -framework Security -framework SystemConfiguration")
    target_link_libraries (calc_executable "-framework CoreFoundation -framework Security -framework SystemConfiguration")
    target_link_libraries (lex_executable  "-framework CoreFoundation -framework Security -framework SystemConfiguration")
endif (DARWIN)

set_property (TARGET task_executable PROPERTY OUTPUT_NAME "task")

install (TARGETS task_executable DESTINATION ${TASK_BINDIR})

set_property (TARGET calc_executable PROPERTY OUTPUT_NAME "calc")
set_property (TARGET lex_executable PROPERTY OUTPUT_NAME "lex")

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
