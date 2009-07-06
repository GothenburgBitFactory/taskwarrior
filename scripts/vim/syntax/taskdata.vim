" Vim syntax file
" Language:	task data
" Maintainer:	John Florian <jflorian@doubledog.org>
"
"
" Copyright 2009 John Florian
" All rights reserved.
"
" This file is part of the task project.
"
" This program is free software; you can redistribute it and/or modify it under
" the terms of the GNU General Public License as published by the Free Software
" Foundation; either version 2 of the License, or (at your option) any later
" version.
"
" This program is distributed in the hope that it will be useful, but WITHOUT
" ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
" FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
" details.
"
" You should have received a copy of the GNU General Public License along with
" this program; if not, write to the
"
"     Free Software Foundation, Inc.,
"     51 Franklin Street, Fifth Floor,
"     Boston, MA
"     02110-1301
"     USA
"
"


" For version 5.x: Clear all syntax items.
" For version 6.x: Quit when a syntax file was already loaded.
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" Key Names for values.
syn keyword taskKey	description due end entry imask mask parent priority
syn keyword taskKey	project recur start status tags uuid
syn match taskKey	"annotation_\d\+"

" Values associated with key names.
"
" Strings
syn region taskString	matchgroup=Normal start=+"+ end=+"+
		    \ 	contains=taskEncoded,taskUUID,@Spell
"
" Special Embedded Characters (e.g., "&comma;")
syn match taskEncoded	"&\a\+;" contained
" UUIDs
syn match taskUUID 	"\x\{8}-\(\x\{4}-\)\{3}\x\{12}" contained


if version >= 508 || !exists("did_taskdata_syntax_inits")
  if version <= 508
    let did_taskdata_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  " The default methods for highlighting.  Can be overridden later.
  HiLink taskEncoded		Function
  HiLink taskKey		Statement
  HiLink taskOperator		Operator
  HiLink taskString 		String
  HiLink taskUUID 		Special

  delcommand HiLink
endif

let b:current_syntax = "taskdata"

" vim:noexpandtab
