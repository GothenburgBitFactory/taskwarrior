cmake_minimum_required (VERSION 3.22...4.0)

find_package(PkgConfig)
if(PkgConfig_FOUND)
	pkg_get_variable(FISH_COMPLETIONSDIR fish completionsdir)
endif()
if(NOT FISH_COMPLETIONSDIR)
	set(FISH_COMPLETIONSDIR share/fish/vendor_completions.d)
	message(STATUS "fish pkgconfig module missing, assumed completions in ${FISH_COMPLETIONSDIR}")
endif()

install (DIRECTORY bash vim hooks
         DESTINATION ${TASK_DOCDIR}/scripts)
install (FILES fish/task.fish
         DESTINATION ${FISH_COMPLETIONSDIR})
install (FILES zsh/_task
         DESTINATION share/zsh/site-functions)
install (DIRECTORY add-ons
         DESTINATION ${TASK_DOCDIR}/scripts
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ             GROUP_EXECUTE
                          WORLD_READ             WORLD_EXECUTE)
