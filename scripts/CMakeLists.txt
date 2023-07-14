cmake_minimum_required (VERSION 3.0)
install (DIRECTORY bash fish vim hooks
         DESTINATION ${CMAKE_INSTALL_DOCDIR}/scripts)
install (FILES zsh/_task
         DESTINATION share/zsh/site-functions)
install (DIRECTORY add-ons
         DESTINATION ${CMAKE_INSTALL_DOCDIR}/scripts
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ             GROUP_EXECUTE
                          WORLD_READ             WORLD_EXECUTE)

