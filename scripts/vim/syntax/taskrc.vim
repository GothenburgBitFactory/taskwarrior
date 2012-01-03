" Vim syntax file
" Language:	support for editing taskwarrior configuration file
" Maintainer:	John Florian <jflorian@doubledog.org>
" Updated:	Sat Feb 20 14:14:44 EST 2010
"
" Copyright 2009 - 2012 John Florian
"
" This file is available under the MIT license.
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

syn keyword taskrcGoodKey	locking detection confirmation next bulk nag weekstart displayweeknumber defaultwidth editor monthsperline annotations _forcecolor debug hooks fontunderline 

syn match taskrcGoodKey	"\(active\|tag\|recurrence\)\.indicator"
syn match taskrcGoodKey	"alias\.\S\{-}="he=e-1
syn match taskrcGoodKey	"calendar\.\(legend\|holidays\|details\(\.report\)\?\)"
syn match taskrcGoodKey	"color\(\.\(alternate\|overdue\|due\(\.today\)\?\|pri\.\([HML]\|none\)\|active\|tagged\|recurring\|header\|footnote\|\(\(tag\|project\|keyword\)\.\S\{-}\)\|debug\|\(calendar\.\(today\|due\(\.today\)\?\|overdue\|weekend\|holiday\|weeknumber\)\)\)\)\?="he=e-1
syn match taskrcGoodKey	"complete\.all\.\(projects\|tags\)"
syn match taskrcGoodKey	"data\.location"
syn match taskrcGoodKey	"dateformat\(\.\(holiday\|report\)\)\?"
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
