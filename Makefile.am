SUBDIRS = src

dist_man_MANS = doc/man/task.1 doc/man/taskrc.5 doc/man/task-tutorial.5 doc/man/task-faq.5 doc/man/task-color.5 doc/man/task-sync.5

#docdir = $(datadir)/doc/${PACKAGE}-${VERSION}
doc_DATA = AUTHORS ChangeLog COPYING NEWS README

EXTRA_DIST = INSTALL

bashscriptsdir = $(docdir)
nobase_dist_bashscripts_DATA = scripts/bash/task_completion.sh

zshscriptsdir = $(docdir)
nobase_dist_zshscripts_DATA = scripts/zsh/_task

vimscriptsdir = $(docdir)
nobase_dist_vimscripts_DATA = scripts/vim/README scripts/vim/ftdetect/task.vim scripts/vim/syntax/taskdata.vim scripts/vim/syntax/taskedit.vim scripts/vim/syntax/taskrc.vim

i18ndir = $(docdir)
nobase_dist_i18n_DATA = i18n/strings.de-DE i18n/strings.en-US i18n/strings.es-ES i18n/strings.fr-FR i18n/strings.nl-NL i18n/strings.sv-SE i18n/tips.de-DE i18n/tips.en-US i18n/tips.sv-SE

rcfiledir = $(docdir)/rc
dist_rcfile_DATA = doc/rc/dark-16.theme doc/rc/dark-256.theme doc/rc/dark-blue-256.theme doc/rc/dark-green-256.theme doc/rc/dark-red-256.theme doc/rc/dark-violets-256.theme doc/rc/dark-yellow-green.theme doc/rc/holidays-AT.rc doc/rc/holidays-CA.rc doc/rc/holidays-DE.rc doc/rc/holidays-DK.rc doc/rc/holidays-ES.rc doc/rc/holidays-FR.rc doc/rc/holidays-IT.rc doc/rc/holidays-NL.rc doc/rc/holidays-NO.rc doc/rc/holidays-NZ.rc doc/rc/holidays-SE.rc doc/rc/holidays-UK.rc doc/rc/holidays-US.rc doc/rc/light-16.theme doc/rc/light-256.theme doc/rc/dark-gray-256.theme
