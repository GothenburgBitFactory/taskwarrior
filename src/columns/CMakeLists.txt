cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (columns_SRCS Column.cpp Column.h
                  ColBg.cpp ColBg.h
                  ColDate.cpp ColDate.h
                  ColDepends.cpp ColDepends.h
                  ColDescription.cpp ColDescription.h
                  ColDue.cpp ColDue.h
                  ColEnd.cpp ColEnd.h
                  ColEntry.cpp ColEntry.h
                  ColFg.cpp ColFg.h
                  ColID.cpp ColID.h
                  ColIMask.cpp ColIMask.h
                  ColMask.cpp ColMask.h
                  ColParent.cpp ColParent.h
                  ColPriority.cpp ColPriority.h
                  ColProject.cpp ColProject.h
                  ColRecur.cpp ColRecur.h
                  ColStart.cpp ColStart.h
                  ColStatus.cpp ColStatus.h
                  ColString.cpp ColString.h
                  ColTags.cpp ColTags.h
                  ColUntil.cpp ColUntil.h
                  ColUrgency.cpp ColUrgency.h
                  ColUUID.cpp ColUUID.h
                  ColWait.cpp ColWait.h)

add_library (columns STATIC ${columns_SRCS})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -ggdb3 -Wall")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")


SET(CMAKE_BUILD_TYPE gcov)
SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
SET(CMAKE_C_FLAGS_GCOV "--coverage")
SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
