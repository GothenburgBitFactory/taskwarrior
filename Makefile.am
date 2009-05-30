SUBDIRS = src

dist_man1_MANS = doc/man1/task.1
dist_man5_MANS = doc/man5/taskrc.5 doc/man5/task-tutorial.5

docdir = $(datadir)/doc/${PACKAGE}-${VERSION}
doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README

bashscriptsdir = $(docdir)
nobase_dist_bashscripts_DATA = scripts/bash/task_completion.sh

zshscriptsdir = $(docdir)
nobase_dist_zshscripts_DATA = scripts/zsh/_task
