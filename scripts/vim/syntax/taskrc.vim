" Vim syntax file
" Language:	support for editing task configuration file
" Maintainer:	John Florian <jflorian@doubledog.org>
" Updated:	Wed Nov 25 12:19:43 EST 2009
"
" Copyright 2009 John Florian
"
" This file is available under the GNU Public License version 2 or later.
" For the full text of this license, see COPYING.


" For version 5.x: Clear all syntax items.
" For version 6.x: Quit when a syntax file was already loaded.
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn match taskrcVal	".\{-}$" 		contains=taskrcComment
syn match taskrcEqual	"="
syn match taskrcKey	"^\s*.\{-}="he=e-1	contains=taskrcEqual

syn keyword taskrcGoodKey	locking curses confirmation next bulk nag dateformat weekstart displayweeknumber due defaultwidth editor monthsperline

syn match taskrcGoodKey	"color\(\.\(overdue\|due\|pri\.\([HML]\|none\)\|active\|tagged\|recurring\|header\|footnote\|\(\(tag\|project\|keyword\)\.\S\{-}\)\)\)\?="he=e-1
syn match taskrcGoodKey	"data\.location"
syn match taskrcGoodKey	"default\.\(command\|project\|priority\)"
syn match taskrcGoodKey	"echo\.command"
syn match taskrcGoodKey	"report\.\S\{-}\.\(description\|columns\|labels\|sort\|filter\)="he=e-1
syn match taskrcGoodKey	"shadow\.\(file\|command\|notify\)"

syn match taskrcComment	"#.*$"

" The default methods for highlighting.  Can be overridden later.
hi def link taskrcComment	Comment
hi def link taskrcKey		Statement
hi def link taskrcVal		String
hi def link taskrcGoodKey	Function

let b:current_syntax = "taskrc"

" vim:noexpandtab
