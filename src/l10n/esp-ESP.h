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

#define STRING_FEEDBACK_NO_MATCH     "Ninguna coincidencia."
#define STRING_FEEDBACK_NO_TASKS     "Ninguna tarea."
#define STRING_FEEDBACK_TASKS_PLURAL "({1} tareas)"
#define STRING_FEEDBACK_TASKS_SINGLE "(1 tarea)"
#define STRING_TASK_DEPEND_CIRCULAR  "Dependencia circular detectada y anulada."
#define STRING_TASK_DEPEND_DUP       "La tarea {1} ya depende de la tarea {2}."
#define STRING_TASK_DEPEND_ITSELF    "Una tarea no puede depender de sí misma."
#define STRING_TASK_DEPEND_MISS_CREA "No se pudo crear una dependencia de la tarea {1} - no encontrada."
#define STRING_TASK_DEPEND_MISS_DEL  "No se pudo eliminar una dependencia de la tarea {1} - no encontrada."
#define STRING_TASK_INVALID_COL_TYPE "Tipo de columna no reconocido '{1}' para la columna '{2}'"
#define STRING_TASK_NO_DESC          "La anotación carece de descripción: {1}"
#define STRING_TASK_NO_ENTRY         "La anotación carece de fecha de entrada: {1}"
#define STRING_TASK_NO_FF1           "Taskwarrior ya no admite el formato de archivo 1, usado originalmente entre el 27 de noviembre de 2006 y el 31 de diciembre de 2007."
#define STRING_TASK_NO_FF2           "Taskwarrior ya no admite el formato de archivo 2, usado originalmente entre el 1 de enero de 2008 y el 12 de abril de 2009."
#define STRING_TASK_NO_FF3           "Taskwarrior ya no admite el formato de archivo 3, usado originalmente entre el 23 de marzo de 2009 y el  16 de mayo de 2009."
#define STRING_TASK_PARSE_UNREC_FF   "Formato de archivo taskwarrior no reconocido."
#define STRING_TASK_SAFETY_ALLOW     "No especificó un filtro, y con el valor de 'allow.empty.filter', no se toma ninguna acción."
#define STRING_TASK_SAFETY_FAIL      "Se impidió la ejecución del comando."
#define STRING_TASK_SAFETY_VALVE     "This command has no filter, and will modify all (including completed and deleted) tasks.  Are you sure?"
#define STRING_TASK_VALID_BEFORE     "Advertencia: ha especificado que la fecha '{1}' es después de la fecha '{2}'."
#define STRING_TASK_VALID_BLANK      "No se puede añadir una tarea que está en blanco."
#define STRING_TASK_VALID_REC_DUE    "Una tarea recurrente debe tener también una fecha de vencimiento."

#endif
