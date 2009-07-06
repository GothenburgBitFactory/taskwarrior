" Vim support file to detect task data files and single task edits
"
" Maintainer:   John Florian <jflorian@doubledog.org>


" for the raw data files
au BufRead,BufNewFile {pending,completed}.data  set filetype=taskdata

" for 'task 42 edit'
au BufRead,BufNewFile *.task                    set filetype=taskedit
