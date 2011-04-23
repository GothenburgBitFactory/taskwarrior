cmake_minimum_required (VERSION 2.8)
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${TASK_DOCDIR}/i18n
                                                FILES_MATCHING REGEX "tips.*"
                                                               PATTERN "CMakeFiles" EXCLUDE)
