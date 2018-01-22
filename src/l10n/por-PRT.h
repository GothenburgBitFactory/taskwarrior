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

#define STRING_FEEDBACK_NO_MATCH     "Nenhuma correspondência."
#define STRING_FEEDBACK_NO_TASKS     "Nenhuma tarefa."
#define STRING_FEEDBACK_TASKS_PLURAL "({1} tarefas)"
#define STRING_FEEDBACK_TASKS_SINGLE "(1 tarefa)"
#define STRING_TASK_DEPEND_CIRCULAR  "Dependência circular detetada e não permitida."
#define STRING_TASK_DEPEND_DUP       "Tarefa {1} já depende da tarefa {2}."
#define STRING_TASK_DEPEND_ITSELF    "Uma tarefa não pode depender dela própria."
#define STRING_TASK_DEPEND_MISS_CREA "Não foi possível criar a dependência da tarefa {1} - esta não existe."
#define STRING_TASK_DEPEND_MISS_DEL  "Não foi possível eliminar a dependência da tarefa {1} - esta não existe."
#define STRING_TASK_INVALID_COL_TYPE "Unrecognized column type '{1}' for column '{2}'"
#define STRING_TASK_NO_DESC          "Descrição da anotação em falta: {1}"
#define STRING_TASK_NO_ENTRY         "Data de entrada da anotação em falta: {1}"
#define STRING_TASK_NO_FF1           "Taskwarrior já não suporta o formato 1, originalmente usado entre 27-Novembro-2006 e 31-Dezembro-2007."
#define STRING_TASK_NO_FF2           "Taskwarrior já não suporta o formato 2, originalmente usado entre 1-Janeiro-2008 e 12-April-2009."
#define STRING_TASK_NO_FF3           "Taskwarrior no longer supports file format 3, originally used between 23 March 2009 and 16 May 2009."
#define STRING_TASK_PARSE_UNREC_FF   "Formato taskwarrior não reconhecido."
#define STRING_TASK_SAFETY_ALLOW     "Não especificou um filtro e com o valor de 'allow.empty.filter', nenhuma ação foi tomada."
#define STRING_TASK_SAFETY_FAIL      "Execução do comando abortada."
#define STRING_TASK_SAFETY_VALVE     "This command has no filter, and will modify all (including completed and deleted) tasks.  Are you sure?"
#define STRING_TASK_VALID_BEFORE     "Aviso: Especificou que a data de '{1}' é posterior à data de '{2}'."
#define STRING_TASK_VALID_BLANK      "Não é possível adicionar uma tarefa em branco."
#define STRING_TASK_VALID_REC_DUE    "Uma tarefa recorrente necessita de uma data de vencimento."

#endif
