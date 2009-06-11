SUBDIRS = src

dist_man_MANS = doc/man/task.1 doc/man/taskrc.5 doc/man/task-tutorial.5

docdir = $(datadir)/doc/${PACKAGE}-${VERSION}
doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README

bashscriptsdir = $(docdir)
nobase_dist_bashscripts_DATA = scripts/bash/task_completion.sh

zshscriptsdir = $(docdir)
nobase_dist_zshscripts_DATA = scripts/zsh/_task

i18ndir = $(docdir)
nobase_dist_i18n_DATA = i18n/strings.de-DE i18n/strings.en-US i18n/strings.es-ES i18n/strings.fr-FR i18n/strings.nl-NL i18n/strings.sv-SE
