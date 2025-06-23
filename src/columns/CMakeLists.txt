cmake_minimum_required (VERSION 3.22...4.0)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${TASK_INCLUDE_DIRS})

set (columns_SRCS Column.cpp Column.h
                  ColDepends.cpp ColDepends.h
                  ColDescription.cpp ColDescription.h
                  ColDue.cpp ColDue.h
                  ColEnd.cpp ColEnd.h
                  ColEntry.cpp ColEntry.h
                  ColID.cpp ColID.h
                  ColIMask.cpp ColIMask.h
                  ColLast.cpp ColLast.h
                  ColMask.cpp ColMask.h
                  ColModified.cpp ColModified.h
                  ColParent.cpp ColParent.h
                  ColProject.cpp ColProject.h
                  ColRecur.cpp ColRecur.h
                  ColRType.cpp ColRType.h
                  ColScheduled.cpp ColScheduled.h
                  ColStart.cpp ColStart.h
                  ColStatus.cpp ColStatus.h
                  ColTags.cpp ColTags.h
                  ColTemplate.cpp ColTemplate.h
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
target_link_libraries(columns taskchampion-cpp)

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
