message ("-- Configuring man pages")
set (man_FILES task-color.5 task-faq.5 task-sync.5 task-tutorial.5 task.1 taskrc.5)
foreach (man_FILE ${man_FILES})
  configure_file (
    man/${man_FILE}.in
    man/${man_FILE})
endforeach (man_FILE)

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man/ DESTINATION share/man/man1
                                                    FILES_MATCHING PATTERN "*.1")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man/ DESTINATION share/man/man5
                                                    FILES_MATCHING PATTERN "*.5")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rc   DESTINATION share/doc/task)
