" Vim syntax file
"
" Copyright (c) 2014 - 2016 Taskwarrior Team
" Copyright (c) 2009 - 2014 John Florian
"
" Permission is hereby granted, free of charge, to any person obtaining a copy
" of this software and associated documentation files (the "Software"), to deal
" in the Software without restriction, including without limitation the rights
" to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
" copies of the Software, and to permit persons to whom the Software is
" furnished to do so, subject to the following conditions:
"
" The above copyright notice and this permission notice shall be included
" in all copies or substantial portions of the Software.
"
" THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
" OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
" FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
" THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
" LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
" OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
" SOFTWARE.
"
" http://www.opensource.org/licenses/mit-license.php
"

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn match taskrcVal ".\{-}$" contains=taskrcComment
syn match taskrcEqual "="
syn match taskrcKey "^\s*.\{-}="he=e-1 contains=taskrcEqual

syn match taskrcGoodKey '^\s*\V_forcecolor='he=e-1
syn match taskrcGoodKey '^\s*\Vabbreviation.minimum='he=e-1
syn match taskrcGoodKey '^\s*\Vactive.indicator='he=e-1
syn match taskrcGoodKey '^\s*\Valias.\S\{-}='he=e-1
syn match taskrcGoodKey '^\s*\Vavoidlastcolumn='he=e-1
syn match taskrcGoodKey '^\s*\Vbulk='he=e-1
syn match taskrcGoodKey '^\s*\Vcalendar.details='he=e-1
syn match taskrcGoodKey '^\s*\Vcalendar.details.report='he=e-1
syn match taskrcGoodKey '^\s*\Vcalendar.holidays='he=e-1
syn match taskrcGoodKey '^\s*\Vcalendar.legend='he=e-1
syn match taskrcGoodKey '^\s*\Vcalendar.offset='he=e-1
syn match taskrcGoodKey '^\s*\Vcalendar.offset.value='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.active='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.alternate='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.blocked='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.blocking='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.burndown.done='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.burndown.pending='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.burndown.started='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.calendar.due='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.calendar.due.today='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.calendar.holiday='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.calendar.overdue='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.calendar.today='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.calendar.weekend='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.calendar.weeknumber='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.debug='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.due='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.due.today='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.error='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.footnote='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.header='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.history.add='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.history.delete='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.history.done='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.overdue='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.uda.priority.H='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.uda.priority.L='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.uda.priority.M='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.recurring='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.scheduled='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.summary.background='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.summary.bar='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.sync.added='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.sync.changed='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.sync.rejected='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.tagged='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.undo.after='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.undo.before='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.until='he=e-1
syn match taskrcGoodKey '^\s*\Vcolor.\(tag\|project\|keyword\|uda\).\S\{-}='he=e-1
syn match taskrcGoodKey '^\s*\Vcolumn.padding='he=e-1
syn match taskrcGoodKey '^\s*\Vcomplete.all.tags='he=e-1
syn match taskrcGoodKey '^\s*\Vconfirmation='he=e-1
syn match taskrcGoodKey '^\s*\Vdata.location='he=e-1
syn match taskrcGoodKey '^\s*\Vdateformat='he=e-1
syn match taskrcGoodKey '^\s*\Vdateformat.annotation='he=e-1
syn match taskrcGoodKey '^\s*\Vdateformat.edit='he=e-1
syn match taskrcGoodKey '^\s*\Vdateformat.holiday='he=e-1
syn match taskrcGoodKey '^\s*\Vdateformat.info='he=e-1
syn match taskrcGoodKey '^\s*\Vdateformat.report='he=e-1
syn match taskrcGoodKey '^\s*\Vdebug='he=e-1
syn match taskrcGoodKey '^\s*\Vdebug.tls='he=e-1
syn match taskrcGoodKey '^\s*\Vdefault.command='he=e-1
syn match taskrcGoodKey '^\s*\Vdefault.due='he=e-1
syn match taskrcGoodKey '^\s*\Vdefault.priority='he=e-1
syn match taskrcGoodKey '^\s*\Vdefault.project='he=e-1
syn match taskrcGoodKey '^\s*\Vdefaultheight='he=e-1
syn match taskrcGoodKey '^\s*\Vdefaultwidth='he=e-1
syn match taskrcGoodKey '^\s*\Vdependency.confirmation='he=e-1
syn match taskrcGoodKey '^\s*\Vdependency.indicator='he=e-1
syn match taskrcGoodKey '^\s*\Vdependency.reminder='he=e-1
syn match taskrcGoodKey '^\s*\Vdetection='he=e-1
syn match taskrcGoodKey '^\s*\Vdisplayweeknumber='he=e-1
syn match taskrcGoodKey '^\s*\Vdom='he=e-1
syn match taskrcGoodKey '^\s*\Vdue='he=e-1
syn match taskrcGoodKey '^\s*\Vexit.on.missing.db='he=e-1
syn match taskrcGoodKey '^\s*\Vexpressions='he=e-1
syn match taskrcGoodKey '^\s*\Vextensions='he=e-1
syn match taskrcGoodKey '^\s*\Vfontunderline='he=e-1
syn match taskrcGoodKey '^\s*\Vgc='he=e-1
syn match taskrcGoodKey '^\s*\Vhyphenate='he=e-1
syn match taskrcGoodKey '^\s*\Vindent.annotation='he=e-1
syn match taskrcGoodKey '^\s*\Vindent.report='he=e-1
syn match taskrcGoodKey '^\s*\Vjournal.info='he=e-1
syn match taskrcGoodKey '^\s*\Vjournal.time='he=e-1
syn match taskrcGoodKey '^\s*\Vjournal.time.start.annotation='he=e-1
syn match taskrcGoodKey '^\s*\Vjournal.time.stop.annotation='he=e-1
syn match taskrcGoodKey '^\s*\Vjson.array='he=e-1
syn match taskrcGoodKey '^\s*\Vlist.all.projects='he=e-1
syn match taskrcGoodKey '^\s*\Vlist.all.tags='he=e-1
syn match taskrcGoodKey '^\s*\Vlocale='he=e-1
syn match taskrcGoodKey '^\s*\Vlocking='he=e-1
syn match taskrcGoodKey '^\s*\Vnag='he=e-1
syn match taskrcGoodKey '^\s*\Vprint.empty.columns='he=e-1
syn match taskrcGoodKey '^\s*\Vrecurrence.indicator='he=e-1
syn match taskrcGoodKey '^\s*\Vrecurrence.limit='he=e-1
syn match taskrcGoodKey '^\s*\Vregex='he=e-1
syn match taskrcGoodKey '^\s*\Vreport.\S\{-}.\(description\|columns\|labels\|sort\|filter\|dateformat\|annotations\)='he=e-1
syn match taskrcGoodKey '^\s*\Vreserved.lines='he=e-1
syn match taskrcGoodKey '^\s*\Vrow.padding='he=e-1
syn match taskrcGoodKey '^\s*\Vrule.precedence.color='he=e-1
syn match taskrcGoodKey '^\s*\Vsearch.case.sensitive='he=e-1
syn match taskrcGoodKey '^\s*\Vtag.indicator='he=e-1
syn match taskrcGoodKey '^\s*\Vtaskd.\(server\|credentials\|certificate\|key\|ca\|trust\|ciphers\)='he=e-1
syn match taskrcGoodKey '^\s*\Vuda.\S\{-}.\(default\|type\|label\|values\)='he=e-1
syn match taskrcGoodKey '^\s*\Vundo.style='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.active.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.age.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.age.max='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.annotations.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.blocked.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.blocking.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.due.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.next.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.uda.priority.H.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.uda.priority.M.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.uda.priority.L.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.project.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.scheduled.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.tags.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.uda.\S\{-}.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.waiting.coefficient='he=e-1
syn match taskrcGoodKey '^\s*\Vurgency.inherit='he=e-1
syn match taskrcGoodKey '^\s*\Vverbose='he=e-1
syn match taskrcGoodKey '^\s*\Vweekstart='he=e-1
syn match taskrcGoodKey '^\s*\Vxterm.title='he=e-1

syn match taskrcComment "#.*$"
syn match taskrcInclude '^\s*include\s'he=e-1

hi def link taskrcComment Comment
hi def link taskrcKey Statement
hi def link taskrcVal String
hi def link taskrcGoodKey Function
hi def link taskrcInclude Include

let b:current_syntax = "taskrc"
