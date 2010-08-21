" Vim support file to detect Taskwarrior data and configuration files and 
" single task edits
"
" Maintainer:   John Florian <jflorian@doubledog.org>
" Updated:	Thu Dec 10 18:28:26 EST 2009
"
" Copyright 2009-2010 John Florian
"
" This file is available under the GNU Public License version 2 or later.
" For the full text of this license, see COPYING.


" Taskwarrior data files
au BufRead,BufNewFile {pending,completed,undo}.data	set filetype=taskdata

" Taskwarrior configuration file
au BufRead,BufNewFile .taskrc				set filetype=taskrc

" Taskwarrior handling of 'task 42 edit'
au BufRead,BufNewFile *.task				set filetype=taskedit

" vim:noexpandtab
