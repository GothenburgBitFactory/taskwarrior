" Vim syntax file
" Language:	support for 'task 42 edit'
" Maintainer:	John Florian <jflorian@doubledog.org>
" Updated:	Wed Jul  8 19:46:32 EDT 2009
"
" Copyright 2009 - 2016 John Florian
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

syn match taskeditHeading	"^\s*#\s*Name\s\+Editable details\s*$" contained
syn match taskeditHeading	"^\s*#\s*-\+\s\+-\+\s*$" contained
syn match taskeditReadOnly	"^\s*#\s*\(UU\)\?ID:.*$" contained
syn match taskeditReadOnly	"^\s*#\s*Status:.*$" contained
syn match taskeditReadOnly	"^\s*#\s*i\?Mask:.*$" contained
syn region taskeditKeyValue	matchgroup=taskeditKey start="^  \S.\{-}:" skip="^\s*#" end="^  \S.\{-}:"me=s-1,he=s-1,re=s-1 contains=taskeditKey,taskeditValue,taskeditComment
syn match taskeditValue	".*$" contained contains=@Spell
syn match taskeditComment	"^\s*#.*$" contains=taskeditReadOnly,taskeditHeading

" The default methods for highlighting.  Can be overridden later.
hi def link taskeditComment	Comment
hi def link taskeditHeading	Function
hi def link taskeditKey	Statement
hi def link taskeditReadOnly	Special
hi def link taskeditValue	String

let b:current_syntax = "taskedit"

" vim:noexpandtab
