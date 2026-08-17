cmake_minimum_required (VERSION 3.22...4.0)

find_package(PkgConfig)
if(PkgConfig_FOUND)
	pkg_get_variable(FISH_COMPLETIONSDIR fish completionsdir)
	pkg_get_variable(BASH_COMPLETIONSDIR bash-completion completionsdir)
endif()
if(NOT FISH_COMPLETIONSDIR)
	set(FISH_COMPLETIONSDIR share/fish/vendor_completions.d)
	message(STATUS "fish pkgconfig module missing, assumed completions in ${FISH_COMPLETIONSDIR}")
endif()
if(NOT BASH_COMPLETIONSDIR)
	set(BASH_COMPLETIONSDIR share/bash-completion/completions)
	message(STATUS "bash-completion pkgconfig module missing, assumed completions in ${BASH_COMPLETIONSDIR}")
endif()

install (DIRECTORY vim hooks
         DESTINATION ${TASK_DOCDIR}/scripts)
install (FILES fish/task.fish
         DESTINATION ${FISH_COMPLETIONSDIR})
install (FILES bash/task.sh
         DESTINATION ${BASH_COMPLETIONSDIR}
         RENAME task)
install (FILES zsh/_task
         DESTINATION share/zsh/site-functions)
install (DIRECTORY add-ons
         DESTINATION ${TASK_DOCDIR}/scripts
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ             GROUP_EXECUTE
                          WORLD_READ             WORLD_EXECUTE)
