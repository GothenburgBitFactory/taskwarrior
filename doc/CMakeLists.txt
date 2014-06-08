cmake_minimum_required (VERSION 2.8)
message ("-- Configuring man pages")
set (man_FILES task-color.5 task-sync.5 taskrc.5 task.1)
foreach (man_FILE ${man_FILES})
  configure_file (
    man/${man_FILE}.in
    man/${man_FILE})
endforeach (man_FILE)

install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man/   DESTINATION ${TASK_MAN1DIR}
                                                      FILES_MATCHING PATTERN "*.1")
install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man/   DESTINATION ${TASK_MAN5DIR}
                                                      FILES_MATCHING PATTERN "*.5")
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rc/    DESTINATION ${TASK_RCDIR})

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/ref/task-ref.pdf DESTINATION ${TASK_DOCDIR})
