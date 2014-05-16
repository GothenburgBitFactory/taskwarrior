cmake_minimum_required (VERSION 2.8)
install (DIRECTORY bash fish vim zsh hooks
         DESTINATION ${TASK_DOCDIR}/scripts)
install (DIRECTORY add-ons
         DESTINATION ${TASK_DOCDIR}/scripts
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ             GROUP_EXECUTE
                          WORLD_READ             WORLD_EXECUTE)

