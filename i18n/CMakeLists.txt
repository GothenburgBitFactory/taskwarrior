install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION share/doc/task/i18n
                                                FILES_MATCHING REGEX "strings.*"
                                                               REGEX "tips.*"
                                                               PATTERN "CMakeFiles" EXCLUDE)
