install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man/ DESTINATION share/man/man1
                                                    FILES_MATCHING PATTERN "*.1")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man/ DESTINATION share/man/man5
                                                    FILES_MATCHING PATTERN "*.5")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rc   DESTINATION share/doc/task)
