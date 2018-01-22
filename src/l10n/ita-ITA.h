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

#define STRING_FEEDBACK_NO_MATCH     "Nessuna corrispondenza."
#define STRING_FEEDBACK_NO_TASKS     "Nessun task."
#define STRING_FEEDBACK_TASKS_PLURAL "({1} task)"
#define STRING_FEEDBACK_TASKS_SINGLE "(1 task)"
#define STRING_TASK_DEPEND_CIRCULAR  "Dipendenza circolare riscontrata ed evitata."
#define STRING_TASK_DEPEND_DUP       "Task {1} già dipende da {2}."
#define STRING_TASK_DEPEND_ITSELF    "Un task non può dipendere da sè stesso."
#define STRING_TASK_DEPEND_MISS_CREA "Impossibile creare la dipendenza dal task {1} - non trovato."
#define STRING_TASK_DEPEND_MISS_DEL  "Impossibile cancellare la dipendenza dal task {1} - non trovato."
#define STRING_TASK_INVALID_COL_TYPE "Unrecognized column type '{1}' for column '{2}'"
#define STRING_TASK_NO_DESC          "Annotazione senza descrizione: {1}"
#define STRING_TASK_NO_ENTRY         "Annotazione senza data di immissione: {1}"
#define STRING_TASK_NO_FF1           "Taskwarrior non supporta più il formato di file 1, usato tra il 27 Novembre 2006 e il 31 Dicembre 2007."
#define STRING_TASK_NO_FF2           "Taskwarrior no longer supports file format 2, originally used between 1 January 2008 and 12 April 2009."
#define STRING_TASK_NO_FF3           "Taskwarrior no longer supports file format 3, originally used between 23 March 2009 and 16 May 2009."
#define STRING_TASK_PARSE_UNREC_FF   "Formato di file non riconosciuto."
#define STRING_TASK_SAFETY_ALLOW     "You did not specify a filter, and with the 'allow.empty.filter' value, no action is taken."
#define STRING_TASK_SAFETY_FAIL      "Prevenuta l'esecuzione del comando."
#define STRING_TASK_SAFETY_VALVE     "This command has no filter, and will modify all (including completed and deleted) tasks.  Are you sure?"
#define STRING_TASK_VALID_BEFORE     "Warning: data '{1}' con valore successivo alla data '{2}'."
#define STRING_TASK_VALID_BLANK      "Impossibile aggiungere un task vuoto."
#define STRING_TASK_VALID_REC_DUE    "Un task periodico deve avere una data di scadenza."

#endif
