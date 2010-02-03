" Vim syntax file
" Language:	support for editing task configuration file
" Maintainer:	John Florian <jflorian@doubledog.org>
" Updated:	Wed Nov 25 12:19:43 EST 2009
"
" Copyright 2009-2010 John Florian
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

syn keyword taskrcGoodKey	locking curses confirmation next bulk nag dateformat weekstart displayweeknumber defaultwidth editor monthsperline annotations _forcecolor blanklines debug hooks fontunderline 

syn match taskrcGoodKey	"alias\.\S\{-}="he=e-1
syn match taskrcGoodKey	"calendar\.\(legend\|holidays\|details\(report\)\?\)"
syn match taskrcGoodKey	"color\(\.\(alternate\|overdue\|due\|pri\.\([HML]\|none\)\|active\|tagged\|recurring\|header\|footnote\|\(\(tag\|project\|keyword\)\.\S\{-}\)\|debug\|\(calendar\.\(today\|due\|overdue\|weekend\|holiday\|weeknumber\)\)\)\)\?="he=e-1
syn match taskrcGoodKey	"complete\.all\.\(projects\|tags\)"
syn match taskrcGoodKey	"data\.location"
syn match taskrcGoodKey	"default\.\(command\|project\|priority\)"
syn match taskrcGoodKey	"due="he=e-1
syn match taskrcGoodKey	"echo\.command"
syn match taskrcGoodKey	"import\.synonym\.\(bg\|description\|due\|end\|entry\|fg\|id\|priority\|project\|recur\|start\|status\|tags\|uuid\)"
syn match taskrcGoodKey	"report\.\S\{-}\.\(description\|columns\|labels\|sort\|filter\|dateformat\|annotations\)="he=e-1
syn match taskrcGoodKey	"search\.case\.sensitive"
syn match taskrcGoodKey	"shadow\.\(file\|command\|notify\)"
syn match taskrcGoodKey	"shell\.prompt"

syn match taskrcComment	"#.*$"

" The default methods for highlighting.  Can be overridden later.
hi def link taskrcComment	Comment
hi def link taskrcKey		Statement
hi def link taskrcVal		String
hi def link taskrcGoodKey	Function

let b:current_syntax = "taskrc"

" vim:noexpandtab
