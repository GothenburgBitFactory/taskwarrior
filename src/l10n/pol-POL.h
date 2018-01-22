////////////////////////////////////////////////////////////////////////////
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

#define STRING_FEEDBACK_NO_MATCH     "Nie znaleziono pasujących."
#define STRING_FEEDBACK_NO_TASKS     "Brak zadań."
#define STRING_FEEDBACK_TASKS_PLURAL "({1} zadania)"
#define STRING_FEEDBACK_TASKS_SINGLE "(1 zadanie)"
#define STRING_TASK_DEPEND_CIRCULAR  "Wykryto i nie dopuszczono do zależności cyklicznej."
#define STRING_TASK_DEPEND_DUP       "Zadanie {1} już jest zależne od zadania {2}."
#define STRING_TASK_DEPEND_ITSELF    "Zadanie nie może zależeć od samego siebie."
#define STRING_TASK_DEPEND_MISS_CREA "Nie można dodać zależności dla zadania {1} - nie znaleziono."
#define STRING_TASK_DEPEND_MISS_DEL  "Nie można usunąć zależności dla zadania {1} - nie znaleziono."
#define STRING_TASK_INVALID_COL_TYPE "Nierozpoznany typ kolumny '{1}' dla kolumny '{2}'"
#define STRING_TASK_NO_DESC          "Komentarz nie posiada treści: {1}"
#define STRING_TASK_NO_ENTRY         "Komentarz nie posiada daty utworzenia: {1}"
#define STRING_TASK_NO_FF1           "Taskwarrior nie wspiera więcej formatu plików 1, wcześniej używanego pomiędzy 27 Listopada 2006 i 31 Grudnia 2007."
#define STRING_TASK_NO_FF2           "Taskwarrior nie wspiera więcej formatu plików 2, wcześniej używanego pomiędzy 1 Stycznia 2008 i 12 Kwietnia 2009."
#define STRING_TASK_NO_FF3           "Taskwarrior nie wspiera więcej formatu plików 3, wcześniej używanego pomiędzy 23 Marca 2009 i 16 Maja 2009."
#define STRING_TASK_PARSE_UNREC_FF   "Nierozpoznany format pliku programu taskwarrior."
#define STRING_TASK_SAFETY_ALLOW     "Brak filtra, z aktywną flagą 'allow.empty.filter', nie podjęto akcji."
#define STRING_TASK_SAFETY_FAIL      "Niedopuszczono do wykonania polecenia."
#define STRING_TASK_SAFETY_VALVE     "This command has no filter, and will modify all (including completed and deleted) tasks.  Are you sure?"
#define STRING_TASK_VALID_BEFORE     "Uwaga: podałeś datę '{1}' jako datę po '{2}'."
#define STRING_TASK_VALID_BLANK      "Nie można dodać pustego zadania."
#define STRING_TASK_VALID_REC_DUE    "Zadanie cykliczne musi posiadać datę zakończenia."

#endif
