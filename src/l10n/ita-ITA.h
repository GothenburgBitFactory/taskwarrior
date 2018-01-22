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

#define STRING_COLUMN_LABEL_RECUR    "Period"
#define STRING_COLUMN_LABEL_TAG      "Tag"
#define STRING_COLUMN_LABEL_UNTIL    "Fino a"
#define STRING_COLUMN_LABEL_URGENCY  "Urgenza"
#define STRING_COLUMN_LABEL_UUID     "UUID"
#define STRING_COLUMN_LABEL_VALUE    "Valore"
#define STRING_FEEDBACK_ANN_ADD      "Annotazione di '{1}' aggiunta."
#define STRING_FEEDBACK_ANN_DEL      "Annotazione '{1}' cancellata."
#define STRING_FEEDBACK_ANN_WAS_MOD  "Annotazione modificata in '{1}'."
#define STRING_FEEDBACK_ATT_DEL      "{1} cancellato."
#define STRING_FEEDBACK_ATT_DEL_DUR  "{1} cancellato (durata: {2})."
#define STRING_FEEDBACK_ATT_MOD      "{1} sarà modificata da '{2}' a '{3}'."
#define STRING_FEEDBACK_ATT_SET      "{1} sarà impostata a '{2}'."
#define STRING_FEEDBACK_ATT_WAS_MOD  "{1} modificata da '{2}' a '{3}'."
#define STRING_FEEDBACK_ATT_WAS_SET  "{1} impostata a '{2}'."
#define STRING_FEEDBACK_BACKLOG      "There is {1} local change.  Sync required."
#define STRING_FEEDBACK_BACKLOG_N    "There are {1} local changes.  Sync required."
#define STRING_FEEDBACK_DELETED      "{1} sarà cancellato."
#define STRING_FEEDBACK_DEP_DEL      "Dipendenze '{1}' cancellate."
#define STRING_FEEDBACK_DEP_MOD      "Le dipendenze saranno modificate da '{1}' in '{2}'."
#define STRING_FEEDBACK_DEP_SET      "Le dipendenze saranno impostate a '{1}'."
#define STRING_FEEDBACK_DEP_WAS_MOD  "Dipendenze cambiate da '{1}' a '{2}'."
#define STRING_FEEDBACK_DEP_WAS_SET  "Dipendenze impostate a '{1}'."
#define STRING_FEEDBACK_EXPIRED      "Il task {1} '{2}' è scaduto ed è stato eliminato"
#define STRING_FEEDBACK_NOP          "Nessuna modifica sarà apportata."
#define STRING_FEEDBACK_NO_MATCH     "Nessuna corrispondenza."
#define STRING_FEEDBACK_NO_TASKS     "Nessun task."
#define STRING_FEEDBACK_TAG_NEXT     "Il tag speciale 'next' aumenterà l'urgenza di questo task in modo che appaia nel report 'next'."
#define STRING_FEEDBACK_TAG_NOCAL    "Il tag speciale 'nocal' manterrà il task fuori dal report 'calendar'."
#define STRING_FEEDBACK_TAG_NOCOLOR  "Il tag speciale 'nocolor' disabilita le regole dei colori per questo task."
#define STRING_FEEDBACK_TAG_NONAG    "Il tag speciale 'nonag' eviterà problemi quando il task è modificato."
#define STRING_FEEDBACK_TAG_VIRTUAL  "Virtual tags (including '{1}') are reserved and may not be added or removed."
#define STRING_FEEDBACK_TASKS_PLURAL "({1} task)"
#define STRING_FEEDBACK_TASKS_SINGLE "(1 task)"
#define STRING_FEEDBACK_UNBLOCKED    "Sbloccato {1} '{2}'."
#define STRING_FEEDBACK_WAS_NOP      "Nessuna modifica apportata."
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
