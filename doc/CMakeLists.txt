#set (man5_FILES taskrc.5 task-color.5 task-faq.5 task-sync.5 task-tutorial.5)
#set (man5_DIR share/man/man5)

#foreach (man_FILE ${man5_FILES})
#  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/${man_FILE}   DESTINATION ${man5_DIR})
#endforeach (man_FILE)

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man/ DESTINATION share/man/man1
                                                    FILES_MATCHING PATTERN "*.1")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man/ DESTINATION share/man/man5
                                                    FILES_MATCHING PATTERN "*.5")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rc   DESTINATION share/doc/task)
