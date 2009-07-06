" Vim support file to detect task data files and single task edits
"
" Maintainer:   John Florian <jflorian@doubledog.org>
"
" Copyright 2009 John Florian
"
" This file is available under the GNU Public License version 2 or later.
" For the full text of this license, see COPYING.


" for the raw data files
au BufRead,BufNewFile {pending,completed}.data  set filetype=taskdata

" for 'task 42 edit'
au BufRead,BufNewFile *.task                    set filetype=taskedit
