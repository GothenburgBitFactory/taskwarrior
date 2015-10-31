cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (columns_SRCS Column.cpp Column.h
                  ColDepends.cpp ColDepends.h
                  ColDescription.cpp ColDescription.h
                  ColDue.cpp ColDue.h
                  ColEnd.cpp ColEnd.h
                  ColEntry.cpp ColEntry.h
                  ColID.cpp ColID.h
                  ColIMask.cpp ColIMask.h
                  ColMask.cpp ColMask.h
                  ColModified.cpp ColModified.h
                  ColParent.cpp ColParent.h
                  ColProject.cpp ColProject.h
                  ColRecur.cpp ColRecur.h
                  ColScheduled.cpp ColScheduled.h
                  ColStart.cpp ColStart.h
                  ColStatus.cpp ColStatus.h
                  ColString.cpp ColString.h
                  ColTags.cpp ColTags.h
                  ColTypeDate.cpp ColTypeDate.h
                  ColTypeDuration.cpp ColTypeDuration.h
                  ColTypeNumeric.cpp ColTypeNumeric.h
                  ColTypeString.cpp ColTypeString.h
                  ColUntil.cpp ColUntil.h
                  ColUrgency.cpp ColUrgency.h
                  ColUDA.cpp ColUDA.h
                  ColUUID.cpp ColUUID.h
                  ColWait.cpp ColWait.h)

add_library (columns STATIC ${columns_SRCS})

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
