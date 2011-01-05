cmake_minimum_required (VERSION 2.8)
include (CheckFunctionExists)

project (task)
set (PROJECT_VERSION "1.9.4")

set (PACKAGE "${PROJECT_NAME}")
set (VERSION "${PROJECT_VERSION}")
set (PACKAGE_BUGREPORT "support@taskwarrior.org")
set (PACKAGE_NAME "${PACKAGE}")
set (PACKAGE_TARNAME "${PACKAGE}")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_STRING "${PACKAGE} ${VERSION}")

find_package (Lua51)
if (LUA51_FOUND)
  set (HAVE_LIBLUA true)
endif (LUA51_FOUND)

find_path (PTHREAD_INCLUDE_DIR pthread.h)
find_library (PTHREAD_LIBRARY NAMES pthread)
if (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)
  message ("-- Found pthread: ${PTHREAD_LIBRARY}")
  set (HAVE_LIBPTHREAD true)
endif (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)

#find_path (READLINE_INCLUDE_DIR readline/readline.h)
#find_library (READLINE_LIBRARY NAMES readline)
#if (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
#  set (HAVE_LIBREADLINE true)
#  set (HAVE_READLINE true)
#endif (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)

set (TASK_INCLUDE_DIRS ${LUA_INCLUDE_DIR}
                       ${PTHREAD_INCLUDE_DIR}
                       ${READLINE_INCLUDE_DIR})

set (TASK_LIBRARIES ${LUA_LIBRARIES}
                    ${PTHREAD_LIBRARY}
                    ${READLINE_LIBRARY})

check_function_exists (random  HAVE_RANDOM)
check_function_exists (srandom HAVE_SRANDOM)
check_function_exists (uuid_unparse_lower HAVE_UUID)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (LINUX true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (DARWIN true)
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (UNKNOWN true)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

configure_file (
  ${CMAKE_SOURCE_DIR}/cmake.h.in
  ${CMAKE_SOURCE_DIR}/auto.h)

add_subdirectory (src)
add_subdirectory (doc)
add_subdirectory (i18n)
add_subdirectory (scripts)
add_subdirectory (test EXCLUDE_FROM_ALL)

set (doc_FILES NEWS ChangeLog README INSTALL AUTHORS COPYING)
foreach (doc_FILE ${doc_FILES})
  install (FILES ${doc_FILE}  DESTINATION share/doc/task)
endforeach (doc_FILE)
