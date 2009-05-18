SUBDIRS = src

MAN_DIST = doc/man1/task.1 doc/man5/taskrc.5
SCRIPTS_DIST = scripts/bash/task_completion.sh scripts/zsh/_task
EXTRA_DIST = $(MAN_DIST) $(SCRIPTS_DIST)

man1_MANS = doc/man1/task.1
man5_MANS = doc/man5/taskrc.5

docdir = $(datadir)/doc/${PACKAGE}-${VERSION}
doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README

scriptsbasedir = $(docdir)/scripts

bashscriptsdir = $(scriptsbasedir)/bash
bashscripts_DATA = scripts/bash/task_completion.sh

zshscriptsdir = $(scriptsbasedir)/zsh
zshscripts_DATA = scripts/zsh/_task
