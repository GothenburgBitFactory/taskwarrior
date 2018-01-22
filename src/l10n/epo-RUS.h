////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2018, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_STRINGS
#define INCLUDED_STRINGS

#define STRING_FEEDBACK_NO_MATCH     "Nenia kongruanto."
#define STRING_FEEDBACK_NO_TASKS     "Nenia tasko."
#define STRING_FEEDBACK_TASKS_PLURAL "({1} taskoj)"
#define STRING_FEEDBACK_TASKS_SINGLE "(1 tasko)"
#define STRING_TASK_DEPEND_CIRCULAR  "Detektis kaj malpermesis ciklan dependecon."
#define STRING_TASK_DEPEND_DUP       "Tasko {1} jam dependas de tasko {2}."
#define STRING_TASK_DEPEND_ITSELF    "Tasko ne povas dependi de si mem."
#define STRING_TASK_DEPEND_MISS_CREA "Ne povis krei dependecon de tasko {1} - netrovita."
#define STRING_TASK_DEPEND_MISS_DEL  "Ne povis viŝi dependecon de tasko {1} - netrovita."
#define STRING_TASK_INVALID_COL_TYPE "Unrecognized column type '{1}' for column '{2}'"
#define STRING_TASK_NO_DESC          "Komento havas mankon de priskribo: {1}"
#define STRING_TASK_NO_ENTRY         "Komento havas mankon de enskrib-dato: {1}"
#define STRING_TASK_NO_FF1           "Taskwarrior ne pli subtenas dosier-aranĝon 1, kiu estis originale uzata inter la 27-a de novembro 2006 kaj la 31-a de decembro 2007."
#define STRING_TASK_NO_FF2           "Taskwarrior ne pli subtenas dosier-aranĝon 2, kiu estis originale uzata inter la 1-a de januaro 2008 kaj la 12-a de aprilo 2009."
#define STRING_TASK_NO_FF3           "Taskwarrior no longer supports file format 3, originally used between 23 March 2009 and 16 May 2009."
#define STRING_TASK_PARSE_UNREC_FF   "Ne povis rekoni taskan dosier-aranĝon."
#define STRING_TASK_SAFETY_ALLOW     "Vi ne specifis filtrilon. Laŭ la valoro 'allow.empty.filter', ne faros nenion."
#define STRING_TASK_SAFETY_FAIL      "Antaŭmalebligis komandon."
#define STRING_TASK_SAFETY_VALVE     "This command has no filter, and will modify all (including completed and deleted) tasks.  Are you sure?"
#define STRING_TASK_VALID_BEFORE     "Averto: Vi specifis daton '{1}' post dato '{2}'."
#define STRING_TASK_VALID_BLANK      "Ne povas krei blankan taskon."
#define STRING_TASK_VALID_REC_DUE    "Reokazanta tasko devas ankaŭ havi daton 'due'."

#endif
