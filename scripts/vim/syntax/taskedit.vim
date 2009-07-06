" Vim syntax file
" Language:	support for 'task 42 edit'
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

syn match taskHeading 	"^\s*#\s*Name\s\+Editable details\s*$" contained
syn match taskHeading 	"^\s*#\s*-\+\s\+-\+\s*$" contained
syn match taskReadOnly 	"^\s*#\s*\(UU\)\?ID:.*$" contained
syn match taskReadOnly 	"^\s*#\s*Status:.*$" contained
syn match taskReadOnly 	"^\s*#\s*i\?Mask:.*$" contained
syn match taskKey	"^ *.\{-}:" nextgroup=taskString
syn match taskComment	"^\s*#.*$" contains=taskReadOnly,taskHeading
syn match taskString	".*$" contained contains=@Spell


if version >= 508 || !exists("did_taskedit_syntax_inits")
  if version <= 508
    let did_taskedit_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  " The default methods for highlighting.  Can be overridden later.
  HiLink taskComment		Comment
  HiLink taskHeading		Function
  HiLink taskKey		Statement
  HiLink taskReadOnly 		Special
  HiLink taskString		String

  delcommand HiLink
endif

let b:current_syntax = "taskedit"

" vim:noexpandtab
