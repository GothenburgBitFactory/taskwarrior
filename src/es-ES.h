////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

////////////////////////////////////////////////////////////////////////////////
//
// This file contains all the strings that should be localized.  If a string is
// *not* in this file, then either:
//   (a) it should not be localized, or
//   (b) you have found a bug - please report it
//
// Strings that should be localized:
//   - text output that the user sees
//
// Strings that should NOT be localized:
//   - .taskrc configuration variable names
//   - command names
//   - extension function names
//   - certain literals associated with parsing
//   - debug strings
//   - attribute names
//   - modifier names
//   - logical operators (and, or, xor)
//
// Rules:
//   - Localized strings should not in general  contain leading or trailing
//     white space, including \n, thus allowing the code to compose strings.
//   - Retain the tense of the original string.
//   - Retain the same degree of verbosity of the original string.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Translators:
//   1. Copy this file (en-US.h) to a new file with the target locale as the
//      file name.  Using German as an example, do this:
//
//        cp en-US.h de-DE.h
//
//   2. Modify all the strings below.
//        i.e. change "Unknown error." to "Unbekannter Fehler.".
//
//   3. Add your new translation to the task.git/src/i18n.h file, if necessary,
//      by inserting:
//
//        #elif PACKAGE_LANGUAGE == LANGUAGE_DE_DE
//        #include <de-DE.h>
//
//   4. Add your new language to task.git/CMakeLists.txt, making sure that
//      number is unique:
//
//        set (LANGUAGE_DE_DE 4)
//
//   5. Add your new language to task.git/cmake.h.in:
//
//        #define LANGUAGE_DE_DE ${LANGUAGE_DE_DE}                                        
//
//   6. Build your localized Taskwarrior with these commands:
//
//      cd task.git
//      cmake -D LANGUAGE=3 .
//      make
//
//   7. Submit your translation to support@taskwarrior.org, where it will be
//      shared with others.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_STRINGS
#define INCLUDED_STRINGS

// Note that for English, the following two lines are not displayed.  For all
// other localizations, these lines will appear in the output of the 'version'
// and 'diagnostics' commands.
//
// DO NOT include a copyright in the translation.
//
#define STRING_LOCALIZATION_DESC     "- localización española"
#define STRING_LOCALIZATION_AUTHOR   "Traducido al español por Fidel Mato."

// A3
#define STRING_A3_ALTERNATE_RC       "Usando el archivo .taskrc alternativo {1}"
#define STRING_A3_ALTERNATE_DATA     "Usando data.location alternativa {1}"
// Sobrescritura: de R.A.E. sobrescribir
#define STRING_A3_OVERRIDE_RC        "Sobrescritura de configuración rc.{1}:{2}"
#define STRING_A3_OVERRIDE_PROBLEM   "Problema con sobrescritura: {1}"
#define STRING_A3_UNKNOWN_ATTMOD     "Error: modificador de atributo no reconocido '{1}'."
#define STRING_A3_MISMATCHED_PARENS  "Paréntesis desemparejados en expresión"
#define STRING_A3_PATTERN_GARBAGE    "Carácter(es) no reconocido(s) al final del patrón."
#define STRING_A3_MALFORMED_PATTERN  "Patrón incorrecto."
#define STRING_A3_MALFORMED_ID       "ID incorrecto."
#define STRING_A3_MALFORMED_UUID     "UUID incorrecto."
#define STRING_A3_ID_AFTER_HYPHEN    "ID no reconocido tras guión."
#define STRING_A3_RANGE_INVERTED     "Rango invertido 'alto-bajo' en vez de 'bajo-alto'"
#define STRING_A3_UUID_AFTER_COMMA   "UUID no reconocido tras coma."

// Color
#define STRING_COLOR_UNRECOGNIZED    "El color '{1}' no se reconoce."

// columns/Col*
#define STRING_COLUMN_BAD_NAME       "Nombre de columna no reconocido '{1}'."
#define STRING_COLUMN_BAD_FORMAT     "Formato de columna no reconocido '{1}.{2}'"
#define STRING_COLUMN_LABEL_TASKS    "Tareas"
#define STRING_COLUMN_LABEL_DEP      "Depende"
#define STRING_COLUMN_LABEL_DEP_S    "Dep"
#define STRING_COLUMN_LABEL_DESC     "Descripción"
#define STRING_COLUMN_LABEL_DUE      "Vencimiento"
#define STRING_COLUMN_LABEL_END      "Fin"
#define STRING_COLUMN_LABEL_ENTERED  "Entrada"
#define STRING_COLUMN_LABEL_COUNT    "Recuento"
#define STRING_COLUMN_LABEL_COMPLETE "Completada"
#define STRING_COLUMN_LABEL_MOD      "Modificada"
#define STRING_COLUMN_LABEL_ADDED    "Añadida"
#define STRING_COLUMN_LABEL_AGE      "Edad"
#define STRING_COLUMN_LABEL_ID       "ID"
#define STRING_COLUMN_LABEL_PRI      "Pri"
#define STRING_COLUMN_LABEL_PRIORITY "Prioridad"
#define STRING_COLUMN_LABEL_PROJECT  "Proyecto"
#define STRING_COLUMN_LABEL_UNTIL    "Hasta"
#define STRING_COLUMN_LABEL_WAIT     "Espera"
#define STRING_COLUMN_LABEL_WAITING  "Esperando hasta"
#define STRING_COLUMN_LABEL_RECUR    "Recur"
#define STRING_COLUMN_LABEL_RECUR_L  "Recurrencia"
#define STRING_COLUMN_LABEL_START    "Comienzo"
#define STRING_COLUMN_LABEL_STARTED  "Comenzada"
#define STRING_COLUMN_LABEL_ACTIVE   "A"
#define STRING_COLUMN_LABEL_STATUS   "Estatus"
#define STRING_COLUMN_LABEL_STAT     "Est"
#define STRING_COLUMN_LABEL_STAT_PE  "Pendientes"
#define STRING_COLUMN_LABEL_STAT_CO  "Completadas"
#define STRING_COLUMN_LABEL_STAT_DE  "Suprimidas"
#define STRING_COLUMN_LABEL_STAT_WA  "Esperando"
// Mejor Periódica, pero STRING_COLUMN_LABEL_STAT_P es Pendiente
#define STRING_COLUMN_LABEL_STAT_RE  "Recurrentes"
#define STRING_COLUMN_LABEL_STAT_P   "P"
#define STRING_COLUMN_LABEL_STAT_C   "C"
#define STRING_COLUMN_LABEL_STAT_D   "S"
#define STRING_COLUMN_LABEL_STAT_W   "E"
#define STRING_COLUMN_LABEL_STAT_R   "R"
#define STRING_COLUMN_LABEL_TAGS     "Marcas"
#define STRING_COLUMN_LABEL_TAG      "Marca"
#define STRING_COLUMN_LABEL_UUID     "UUID"
#define STRING_COLUMN_LABEL_URGENCY  "Urgencia"
#define STRING_COLUMN_LABEL_NAME     "Nombre"
#define STRING_COLUMN_LABEL_VALUE    "Valor"
#define STRING_COLUMN_LABEL_MASK     "Máscara"
#define STRING_COLUMN_LABEL_MASK_IDX "Máscara de Índice"
#define STRING_COLUMN_LABEL_PARENT   "Tarea madre"
#define STRING_COLUMN_LABEL_DATE     "Fecha"
#define STRING_COLUMN_LABEL_COLUMN   "Columnas"
#define STRING_COLUMN_LABEL_STYLES   "Formatos soportados"
#define STRING_COLUMN_LABEL_EXAMPLES "Ejemplo"
#define STRING_COLUMN_LABEL_SCHED    "Programada"
#define STRING_COLUMN_LABEL_UDA      "Nombre"
#define STRING_COLUMN_LABEL_TYPE     "Tipo"
#define STRING_COLUMN_LABEL_LABEL    "Etiqueta"
#define STRING_COLUMN_LABEL_DEFAULT  "Defecto"
#define STRING_COLUMN_LABEL_VALUES   "Valores permitidos"
#define STRING_COLUMN_LABEL_UDACOUNT "Recuento de uso"
#define STRING_COLUMN_LABEL_ORPHAN   "UDA huérfano"

// Column Examples
#define STRING_COLUMN_EXAMPLES_TAGS  "casa @tarea"
#define STRING_COLUMN_EXAMPLES_PROJ  "casa.jardin"
#define STRING_COLUMN_EXAMPLES_PAR   "casa"
#define STRING_COLUMN_EXAMPLES_IND   "  casa.jardin"
#define STRING_COLUMN_EXAMPLES_DESC  "Mueve tu ropa a la percha de abajo"
#define STRING_COLUMN_EXAMPLES_ANNO1 "Inmediatamente antes de comer"
#define STRING_COLUMN_EXAMPLES_ANNO2 "Si juegas en el partido de esta tarde"
#define STRING_COLUMN_EXAMPLES_ANNO3 "Antes de que escribas a casa"
#define STRING_COLUMN_EXAMPLES_ANNO4 "Si no te estás cortando el pelo"

// commands/Cmd*
#define STRING_CMD_CONFLICT          "informe personalizado '{1}' en conflicto con comando interno de taskwarrior."
#define STRING_CMD_VERSION_USAGE     "Muestra el número de versión de taskwarrior"
#define STRING_CMD_VERSION_USAGE2    "Muestra el número de versión (solamente) de taskwarrior"
#define STRING_CMD_VERSION_MIT       "Taskwarrior solamente se puede copiar bajo los términos de la licencia MIT, que se pueden encontrar en el conjunto de código fuente de taskwarrior."

#define STRING_CMD_VERSION_DOCS      "La documentación de taskwarrior se puede consultar usando 'man task', 'man taskrc', 'man task-tutorial', 'man task-color', 'man task-sync' y 'man task-faq' o en http://taskwarrior.org"
#define STRING_CMD_VERSION_BUILT     "{1} {2} construido para "
#define STRING_CMD_VERSION_UNKNOWN   "desconocido"
#define STRING_CMD_VERSION_COPY      "Copyright (C) 2006 - 2013 P. Beckingham, F. Hernandez."
#define STRING_CMD_LOGO_USAGE        "Muestra el logo de Taskwarrior"
#define STRING_CMD_LOGO_COLOR_REQ    "El comando logo requiere que el soporte de color esté habilitado."
#define STRING_CMD_EXEC_USAGE        "Ejecuta comandos y scripts externos"
#define STRING_CMD_URGENCY_USAGE     "Muestra la medida de urgencia de una tarea"
#define STRING_CMD_URGENCY_RESULT    "tarea {1} urgencia {2}"
#define STRING_CMD_ADD_USAGE         "Añade una nueva tarea"
#define STRING_CMD_ADD_FEEDBACK      "Creada tarea {1}."
#define STRING_CMD_ADD_BAD_ATTRIBUTE "Atributo '{1}' no reconocido."
#define STRING_CMD_LOG_USAGE         "Añade una nueva tarea que ya ha sido completada"
#define STRING_CMD_LOG_NO_RECUR      "No puede registrar tareas recurrentes."
#define STRING_CMD_LOG_NO_WAITING    "No puede registrar tareas en espera."
#define STRING_CMD_LOG_LOGGED        "Tarea registrada."
#define STRING_CMD_IDS_USAGE_RANGE   "Muestra los IDs de las tareas coincidentes, como un rango"
#define STRING_CMD_IDS_USAGE_LIST    "Muestra los IDs de las tareas coincidentes, en forma de lista"
#define STRING_CMD_IDS_USAGE_ZSH     "Muestra los IDs y descripciones de las tareas coincidentes"
#define STRING_CMD_UDAS_USAGE        "Muestra detalles de todos los UDA definidos"
#define STRING_CMD_UDAS_COMPL_USAGE  "Muestra los UDAs definidos con fines de terminación"
#define STRING_CMD_UUIDS_USAGE_RANGE "Muestra los UUIDs de las tareas coincidentes, como una lista separada por comas"
#define STRING_CMD_UUIDS_USAGE_LIST  "Muestra los UUIDs de las tareas coincidentes, como una lista"
#define STRING_CMD_UUIDS_USAGE_ZSH   "Muestra los UUIDs y descripciones de las tareas coincidentes"
#define STRING_CMD_EXPORT_USAGE      "Exporta tareas en formato JSON"
// metadatos: RAE 3ªed
#define STRING_CMD_INFO_USAGE        "Muestra todos los datos y metadatos"
#define STRING_CMD_INFO_BLOCKED      "Esta tarea está bloqueada por"
#define STRING_CMD_INFO_BLOCKING     "Esta tarea bloquea"
#define STRING_CMD_INFO_UNTIL        "Hasta"
#define STRING_CMD_INFO_MODIFICATION "Modificación"
#define STRING_CMD_INFO_TOTAL_ACTIVE "Tiempo total activa"
#define STRING_CMD_INFO_MODIFIED     "Modificada por última vez"
#define STRING_CMD_UNDO_USAGE        "Revierte el cambio más reciente a una tarea"
#define STRING_CMD_UNDO_MODS         "El comando undo no permite más modificación a la tarea."
#define STRING_CMD_STATS_USAGE       "Muestra estadísticas de la base de datos de tareas"
#define STRING_CMD_STATS_CATEGORY    "Categoría"
#define STRING_CMD_STATS_DATA        "Datos"
#define STRING_CMD_STATS_TOTAL       "Total"
#define STRING_CMD_STATS_ANNOTATIONS "Anotaciones"
#define STRING_CMD_STATS_UNIQUE_TAGS "Marcas únicas"
#define STRING_CMD_STATS_PROJECTS    "Proyectos"
#define STRING_CMD_STATS_DATA_SIZE   "Tamaño de datos"
#define STRING_CMD_STATS_UNDO_TXNS   "Transacciones deshacer"
#define STRING_CMD_STATS_BACKLOG     "Sincroniza transacciones atrasadas"
#define STRING_CMD_STATS_TAGGED      "Tareas marcadas"
#define STRING_CMD_STATS_OLDEST      "Tarea más antigua"
#define STRING_CMD_STATS_NEWEST      "Tarea más reciente"
//#define STRING_CMD_STATS_USED_FOR    "Tarea usada por"
#define STRING_CMD_STATS_USED_FOR    "Tarea en uso durante"
#define STRING_CMD_STATS_ADD_EVERY   "Tarea añadida cada"
#define STRING_CMD_STATS_COMP_EVERY  "Tarea completada cada"
#define STRING_CMD_STATS_DEL_EVERY   "Tarea suprimida cada"
#define STRING_CMD_STATS_AVG_PEND    "Tiempo medio pendiente"

#define STRING_CMD_STATS_DESC_LEN    "Longitud media de desc"
#define STRING_CMD_STATS_CHARS       "{1} caracteres"
#define STRING_CMD_STATS_LAST_SYNC   "Ültima sincronización del servidor"
#define STRING_CMD_STATS_BLOCKED     "Tareas bloqueadas"
#define STRING_CMD_STATS_BLOCKING    "Tareas bloqueantes"
#define STRING_CMD_REPORTS_USAGE     "Lista todos los informes soportados"
#define STRING_CMD_REPORTS_REPORT    "Informe"
#define STRING_CMD_REPORTS_DESC      "Descripción"
#define STRING_CMD_REPORTS_SUMMARY   "{1} informes"
//#define STRING_CMD_REPORTS_SUMMARY   "{1} informa"
#define STRING_CMD_TAGS_USAGE        "Muestra una lista de todas las marcas en uso"
#define STRING_CMD_COMTAGS_USAGE     "Muestra una lista de todas las marcas (solo nombres) en uso, con fines de auto-completado"
#define STRING_CMD_TAGS_SINGLE       "1 marca"
#define STRING_CMD_TAGS_PLURAL       "{1} marcas"
#define STRING_CMD_TAGS_NO_TAGS      "No hay marcas."
#define STRING_CMD_HISTORY_USAGE_M   "Muestra un informe de historia de tareas, por mes"

#define STRING_CMD_HISTORY_YEAR      "Año"
#define STRING_CMD_HISTORY_MONTH     "Mes"
#define STRING_CMD_HISTORY_ADDED     "Añadidas"
#define STRING_CMD_HISTORY_COMP      "Completadas"
#define STRING_CMD_HISTORY_DEL       "Suprimidas"
#define STRING_CMD_HISTORY_NET       "Netas"
#define STRING_CMD_HISTORY_USAGE_A   "Muestra un informe de historia de tareas, por año"
#define STRING_CMD_HISTORY_AVERAGE   "Media"
#define STRING_CMD_HISTORY_LEGEND    "Leyenda: {1}, {2}, {3}"
#define STRING_CMD_HISTORY_LEGEND_A  "Leyenda: + añadida, X completada, - suprimida"

#define STRING_CMD_GHISTORY_USAGE_M  "Muestra un informe gráfico de historia de tareas, por mes"
#define STRING_CMD_GHISTORY_USAGE_A  "Muestra un informe gráfico de historia de tareas, por año"
#define STRING_CMD_GHISTORY_YEAR     "Año"
#define STRING_CMD_GHISTORY_MONTH    "Mes"
#define STRING_CMD_GHISTORY_NUMBER   "Número Añadidas/Completadas/Suprimidas"

#define STRING_CMD_DONE_USAGE        "Marca la tarea especificada como completada"
#define STRING_CMD_DONE_CONFIRM      "¿Completar tarea {1} '{2}'?"
#define STRING_CMD_DONE_TASK         "Completada tarea {1} '{2}'."
#define STRING_CMD_DONE_NO           "Tarea no completada."
#define STRING_CMD_DONE_NOTPEND      "La tarea {1} '{2}' no está pendiente ni esperando."
#define STRING_CMD_DONE_1            "Completada {1} tarea."
#define STRING_CMD_DONE_N            "Completadas {1} tareas."

#define STRING_CMD_PROJECTS_USAGE    "Muestra todos los nombres de proyecto en uso"
#define STRING_CMD_PROJECTS_USAGE_2  "Muestra una lista de todos los nombres (solamente) de proyecto en uso"
#define STRING_CMD_PROJECTS_NO       "No hay proyectos."
#define STRING_CMD_PROJECTS_PRI_N    "Pri:None"
#define STRING_CMD_PROJECTS_PRI_H    "Pri:H"
#define STRING_CMD_PROJECTS_PRI_M    "Pri:M"
#define STRING_CMD_PROJECTS_PRI_L    "Pri:L"
#define STRING_CMD_PROJECTS_NONE     "(ninguno)"
#define STRING_CMD_PROJECTS_SUMMARY  "{1} proyecto"
#define STRING_CMD_PROJECTS_SUMMARY2 "{1} proyectos"
#define STRING_CMD_PROJECTS_TASK     "({1} tarea)"
#define STRING_CMD_PROJECTS_TASKS    "({1} tareas)"
#define STRING_CMD_SUMMARY_USAGE     "Muestra un informe de estatus de tareas por proyecto"
#define STRING_CMD_SUMMARY_PROJECT   "Proyecto"
#define STRING_CMD_SUMMARY_REMAINING "Quedan"
#define STRING_CMD_SUMMARY_AVG_AGE   "Edad media"
#define STRING_CMD_SUMMARY_COMPLETE  "Completas"
#define STRING_CMD_SUMMARY_NONE      "(ninguna)"
#define STRING_CMD_COUNT_USAGE       "Cuenta tareas que coinciden"

#define STRING_CMD_UDAS_NO           "Ningún UDA definido."
#define STRING_CMD_UDAS_SUMMARY      "{1} UDA definido"
#define STRING_CMD_UDAS_SUMMARY2     "{1} UDAs definidos"
#define STRING_CMD_UDAS_ORPHAN       "{1} UDA huérfano"
#define STRING_CMD_UDAS_ORPHANS      "{1} UDAs huérfanos"

#define STRING_CMD_DELETE_USAGE      "Suprime la tarea especificada"
#define STRING_CMD_DELETE_CONFIRM    "¿Suprimir permanentemente la tarea {1} '{2}'?"
#define STRING_CMD_DELETE_TASK       "Suprimiendo tarea {1} '{2}'."
#define STRING_CMD_DELETE_TASK_R     "Suprimiendo tarea recurrente {1} '{2}'."
#define STRING_CMD_DELETE_CONFIRM_R  "Esta es una tarea recurrente. ¿Quiere suprimir todas las recurrencias pendientes de esta misma tarea?"
#define STRING_CMD_DELETE_NO         "Tarea no suprimida."
#define STRING_CMD_DELETE_NOT_DEL    "La tarea {1} '{2}' no es suprimible."
#define STRING_CMD_DELETE_1          "Suprimida {1} tarea."
#define STRING_CMD_DELETE_N          "Suprimidas {1} tareas."

#define STRING_CMD_DUPLICATE_USAGE   "Duplica las tareas especificadas"
#define STRING_CMD_DUPLICATE_REC     "Nota: la tarea {1} era una tarea madre recurrente. La tarea duplicada también lo es."
#define STRING_CMD_DUPLICATE_NON_REC "Nota: la tarea {1} era una tarea recurrente. La tarea duplicada no lo es."
#define STRING_CMD_DUPLICATE_CONFIRM "¿Duplicar tarea {1} '{2}'?"
#define STRING_CMD_DUPLICATE_TASK    "Duplicada tarea {1} '{2}'."
#define STRING_CMD_DUPLICATE_NO      "Tarea no duplicada."
#define STRING_CMD_DUPLICATE_1       "Duplicada {1} tarea."
#define STRING_CMD_DUPLICATE_N       "Duplicadas {1} tareas."

#define STRING_CMD_START_USAGE       "Marca la tarea especificada como comenzada"
#define STRING_CMD_START_NO          "Tarea no comenzada."
#define STRING_CMD_START_ALREADY     "Tarea {1} '{2}' ya comenzada."
#define STRING_CMD_START_TASK        "Comenzando tarea {1} '{2}'."
#define STRING_CMD_START_CONFIRM     "¿Comenzar tarea {1} '{2}'?"
#define STRING_CMD_START_1           "Comenzada {1} tarea."
#define STRING_CMD_START_N           "Comenzadas {1} tareas."

#define STRING_CMD_STOP_USAGE        "Elimina el tiempo de 'comienzo' de una tarea"

#define STRING_CMD_STOP_NO           "Tarea no detenida."
#define STRING_CMD_STOP_ALREADY      "Tarea {1} '{2}' no comenzada."
#define STRING_CMD_STOP_TASK         "Deteniendo tarea {1} '{2}'."
#define STRING_CMD_STOP_CONFIRM      "¿Detener tarea {1} '{2}'?"
#define STRING_CMD_STOP_1            "Detenida {1} tarea."
#define STRING_CMD_STOP_N            "Detenidas {1} tareas."

#define STRING_CMD_APPEND_USAGE      "Añade texto al final de la descripción de una tarea existente"
#define STRING_CMD_APPEND_1          "Añadiendo texto a {1} tarea."
#define STRING_CMD_APPEND_N          "Añadiendo texto a {1} tareas."
#define STRING_CMD_APPEND_TASK       "Añadiendo texto a la tarea {1} '{2}'."
#define STRING_CMD_APPEND_TASK_R     "Añadiendo texto a la tarea recurrente {1} '{2}'."
#define STRING_CMD_APPEND_CONFIRM_R  "Esta es una tarea recurrente. ¿Quiere añadir a todas las recurrencias pendientes de esta misma tarea?"
#define STRING_CMD_APPEND_CONFIRM    "¿Añadir a la tarea {1} '{2}'?"
#define STRING_CMD_APPEND_NO         "Texto no añadido a la tarea."

#define STRING_CMD_PREPEND_USAGE     "Antepone texto a la descripción de una tarea existente"
#define STRING_CMD_PREPEND_1         "Antepuesto texto a {1} tarea."
#define STRING_CMD_PREPEND_N         "Antepuesto texto a {1} tareas."
#define STRING_CMD_PREPEND_TASK      "Anteponiendo texto a la tarea {1} '{2}'."
#define STRING_CMD_PREPEND_TASK_R    "Antepuesto texto a la tarea recurrente {1} '{2}'."
#define STRING_CMD_PREPEND_CONFIRM_R "Esta es una tarea recurrente. ¿Quiere anteponer texto a todas las recurrencias pendientes de esta misma tarea?"
#define STRING_CMD_PREPEND_CONFIRM   "¿Anteponer texto a la tarea {1} '{2}'?"
#define STRING_CMD_PREPEND_NO        "Texto no antepuesto."

#define STRING_CMD_ANNO_USAGE        "Añade una anotación a una tarea existente"
#define STRING_CMD_ANNO_CONFIRM      "¿Anotar la tarea {1} '{2}'?"
#define STRING_CMD_ANNO_TASK         "Anotando la tarea {1} '{2}'."
#define STRING_CMD_ANNO_TASK_R       "Anotando la tarea recurrente {1} '{2}'."
#define STRING_CMD_ANNO_CONFIRM_R    "Esta es una tarea recurrente. ¿Quiere anotar todas las recurrencias pendientes de esta misma tarea?"
#define STRING_CMD_ANNO_NO           "Tarea no anotada."
#define STRING_CMD_ANNO_1            "Anotada {1} tarea."
#define STRING_CMD_ANNO_N            "Anotadas {1} tareas."

#define STRING_CMD_COLUMNS_USAGE     "Todas las columnas y estilos de formato soportados"
#define STRING_CMD_COLUMNS_NOTE      "* Significa formato por defecto, y por lo tanto opcional. Por ejemplo 'due' y 'due.formatted' son equivalentes."
#define STRING_CMD_COLUMNS_USAGE2    "Muestra una lista de columnas (solo nombres) soportadas"
#define STRING_CMD_COLUMNS_ARGS      "Solo puede especificar un término de búsqueda."

#define STRING_CMD_DENO_USAGE        "Elimina una anotación"
#define STRING_CMD_DENO_WORDS        "Se debe proporcionar un patrón de anotación."
#define STRING_CMD_DENO_NONE         "La tarea especificada no tiene anotaciones que puedan ser eliminadas."
#define STRING_CMD_DENO_CONFIRM      "¿Desanotar la tarea {1} '{2}'?"
#define STRING_CMD_DENO_FOUND        "Encontrada anotación '{1}' y eliminada."
#define STRING_CMD_DENO_NOMATCH      "No se encontró ninguna anotación coincidente para eliminar en '{1}'."
#define STRING_CMD_DENO_NO           "Tarea no desanotada."
#define STRING_CMD_DENO_1            "Desanotada {1} tarea."
#define STRING_CMD_DENO_N            "Desanotadas {1} tareas."

#define STRING_CMD_IMPORT_USAGE      "Importa archivos JSON"
#define STRING_CMD_IMPORT_SUMMARY    "Importadas {1} tareas."
#define STRING_CMD_IMPORT_NOFILE     "Debe especificar un archivo a importar."
#define STRING_CMD_IMPORT_FILE       "Importando '{1}'"
#define STRING_CMD_IMPORT_NOT_JSON   "No es un objeto JSON: {1}"
#define STRING_TASK_NO_DESC          "La anotación carece de descripción: {1}"
#define STRING_TASK_NO_ENTRY         "La anotación carece de fecha de entrada: {1}"
#define STRING_CMD_SHELL_HELP1       "Escriba un comando de task (como 'list'), o pulse 'Enter'."
#define STRING_CMD_SHELL_HELP2       "No hace falta incluir el propio comando 'task'."
#define STRING_CMD_SHELL_HELP3       "Escriba 'quit' (o 'bye', 'exit') para terminar la sesión."

#define STRING_CMD_SYNC_USAGE        "Sincroniza datos con el Servidor Task"
#define STRING_CMD_SYNC_NO_SERVER    "El Servidor Task no está configurado."
#define STRING_CMD_SYNC_BAD_CRED     "Credenciales del Servidor Task incorrectas."
#define STRING_CMD_SYNC_BAD_CERT     "Task Server certificate missing."
#define STRING_CMD_SYNC_ADD          "   añade {1} '{2}'"
#define STRING_CMD_SYNC_MOD          "modifica {1} '{2}'"
#define STRING_CMD_SYNC_PROGRESS     "Sincronizando con {1}"
#define STRING_CMD_SYNC_SUCCESS0     "Sincronización correcta."
#define STRING_CMD_SYNC_SUCCESS1     "Sincronización correcta. {1} cambios enviados."
#define STRING_CMD_SYNC_SUCCESS2     "Sincronización correcta. {1} cambios recibidos."
#define STRING_CMD_SYNC_SUCCESS3     "Sincronización correcta. {1} cambios enviados, {2} cambios recibidos."
#define STRING_CMD_SYNC_SUCCESS_NOP  "Sincronización correcta. No hay cambios"
#define STRING_CMD_SYNC_FAIL_ACCOUNT "Sincronización fallida. O bien sus credenciales son incorrectas, o su cuenta de Servidor Task no está habilitada."
#define STRING_CMD_SYNC_FAIL_ERROR   "Sincronización fallida. El Servidor Task devolvió error: {1} {2}"
#define STRING_CMD_SYNC_FAIL_CONNECT "Sincronización fallida. No se pudo conectar con el Servidor Task."
#define STRING_CMD_SYNC_BAD_SERVER   "Sincronización fallida. Ajuste de configuración '{1}' incorrecto"
#define STRING_CMD_SYNC_NO_TLS       "Taskwarrior was built without GnuTLS support.  Sync is not available."

#define STRING_CMD_DIAG_USAGE        "Detalles de plataforma, construcción y entorno"
#define STRING_CMD_DIAG_PLATFORM     "Plataforma"
#define STRING_CMD_DIAG_UNKNOWN      "<desconocido>"
#define STRING_CMD_DIAG_COMPILER     "Compilador"
#define STRING_CMD_DIAG_VERSION      "Versión"
#define STRING_CMD_DIAG_CAPS         "Capacidades"
#define STRING_CMD_DIAG_FEATURES     "Características Constructivas"
#define STRING_CMD_DIAG_BUILT        "Construido"
#define STRING_CMD_DIAG_COMMIT       "Commit"
#define STRING_CMD_DIAG_FOUND        "(encontrado)"
#define STRING_CMD_DIAG_MISSING      "(no encontrado)"
#define STRING_CMD_DIAG_ENABLED      "Habilitado"
#define STRING_CMD_DIAG_DISABLED     "deshabilitado"
#define STRING_CMD_DIAG_CONFIG       "Configuración"
#define STRING_CMD_DIAG_EXTERNAL     "Utilidades externas"
#define STRING_CMD_DIAG_TESTS        "Comprobaciones"
#define STRING_CMD_DIAG_UUID_GOOD    "1000 UUIDs únicos generados."
#define STRING_CMD_DIAG_UUID_BAD     "Fallo - UUID duplicado en la iteración {1}"
#define STRING_CMD_DIAG_UUID_SCAN    "Exploradas {1} tareas buscando UUIDs duplicados:"
#define STRING_CMD_DIAG_UUID_DUP     "Encontrado duplicado {1}"
#define STRING_CMD_DIAG_UUID_NO_DUP  "No se encontraron duplicados"
#define STRING_CMD_DIAG_NONE         "-ninguno-"
#define STRING_CMD_PUSH_USAGE        "Envía los archivos locales al URL"
#define STRING_CMD_PUSH_SAME         "No se puede enviar archivos cuando origen y destino son los mismos."
#define STRING_CMD_PUSH_NONLOCAL     "El uri '{1}' no es un directorio local."
#define STRING_CMD_PUSH_TRANSFERRED  "Tareas locales transferidas a {1}"
#define STRING_CMD_PUSH_NO_URI       "No se especificó un uri pera el envío. Especifique el uri de un directorio .task remoto, o cree una entrada 'push.default.uri' en su archivo .taskrc ."
#define STRING_CMD_PULL_USAGE        "Trae archivos remotos desde el URL"
#define STRING_CMD_PULL_SAME         "No se puede traer archivos cuando origen y destino son los mismos."
#define STRING_CMD_PULL_TRANSFERRED  "Tareas locales transferidas desde {1}"
#define STRING_CMD_PULL_NO_URI       "No se especificó un uri desde el que traer. Especifique el uri de un directorio .task remoto, o cree una entrada 'pull.default.uri' en su archivo .taskrc ."
#define STRING_CMD_PULL_MISSING      "Al menos uno de los archivos de base de datos en '{1}' no está presente."
#define STRING_CMD_PULL_NOT_DIR      "El uri '{1}' no es un directorio. ¿Olvidó un '/' final?"
#define STRING_CMD_HCOMMANDS_USAGE   "Genera una lista de todos los comandos, con fines de auto-completado"
#define STRING_CMD_ZSHCOMMANDS_USAGE "Genera una lista de todos los comandos, con fines de auto-completado zsh"
#define STRING_CMD_ALIASES_USAGE     "Genera una lista de todos los alias, con fines de auto-completado"
#define STRING_CMD_INSTALL_USAGE     "Instala extensiones y scripts externos"

#define STRING_CMD_MODIFY_USAGE1     "Modifica la tarea existente con los argumentos provistos."
//#define STRING_CMD_MODIFY_USAGE1     "Modifies the existing task with provided arguments."
#define STRING_CMD_MODIFY_NO_DUE     "No puede especificar una tarea recurrente sin una fecha de vencimiento."
#define STRING_CMD_MODIFY_REM_DUE    "No puede eliminar la fecha de vencimiento de una tarea recurrente."
#define STRING_CMD_MODIFY_REC_ALWAYS "No puede eliminar la recurrencia de una tarea recurrente."
#define STRING_CMD_MODIFY_TASK       "Modificando tarea {1} '{2}'."
#define STRING_CMD_MODIFY_TASK_R     "Modificando tarea recurrente {1} '{2}'."
#define STRING_CMD_MODIFY_1          "Modificada {1} tarea."
#define STRING_CMD_MODIFY_N          "Modificada {1} tareas."
#define STRING_CMD_MODIFY_NO         "Tarea no modificada."
#define STRING_CMD_MODIFY_CONFIRM    "¿Modificar tarea {1} '{2}'?"
#define STRING_CMD_MODIFY_RECUR      "Esta es una tarea recurrente. ¿Quiere modificar todas las recurrencias pendientes de esta misma tarea?"
#define STRING_CMD_MODIFY_NEED_TEXT  "Se debe proveer texto adicional."
//#define STRING_CMD_MODIFY_NEED_TEXT  "Additional text must be provided."  COMPROBAR adicional

#define STRING_CMD_COLOR_USAGE       "Todos los colores, una muestra, o una leyenda"
#define STRING_CMD_COLOR_HERE        "Aquí están los colores en uso actualmente:"
#define STRING_CMD_COLOR_COLOR       "Color"
#define STRING_CMD_COLOR_DEFINITION  "Definición"
#define STRING_CMD_COLOR_EXPLANATION "Use este comando para ver como los colores son mostrados por su terminal."
#define STRING_CMD_COLOR_16          "Uso de 16-colores (soporta subrayado, negrita, fondo brillante):"
#define STRING_CMD_COLOR_256         "Uso de 256-colores (soporta subrayado):"
#define STRING_CMD_COLOR_YOURS       "Su muestra:"
#define STRING_CMD_COLOR_BASIC       "Colores básicos"
#define STRING_CMD_COLOR_EFFECTS     "Efectos"
#define STRING_CMD_COLOR_CUBE        "Cubo de color rgb"
#define STRING_CMD_COLOR_RAMP        "Rampa de grises"
#define STRING_CMD_COLOR_TRY         "Intente ejecutar '{1}'."
//#define STRING_CMD_COLOR_TRY         "Try running '{1}'."
#define STRING_CMD_COLOR_OFF         "El color está actualmente desactivado en su archivo .taskrc . Para activar el color elimine la línea 'color=off', o cambie el 'off' a 'on'."
#define STRING_CMD_CONFIG_USAGE      "Cambia los ajustes en la configuración de task"
#define STRING_CMD_CONFIG_CONFIRM    "¿Está seguro de querer cambiar el valor de '{1}' de '{2}' a '{3}'?"
#define STRING_CMD_CONFIG_CONFIRM2   "¿Está seguro de querer añadir '{1}' con un valor de '{2}'?"
#define STRING_CMD_CONFIG_CONFIRM3   "¿Está seguro de querer eliminar '{1}'?"
#define STRING_CMD_CONFIG_NO_ENTRY   "No se encontró una entrada llamada '{1}'."
#define STRING_CMD_CONFIG_FILE_MOD   "Archivo de configuración {1} modificado."
#define STRING_CMD_CONFIG_NO_CHANGE  "No se hicieron cambios."
#define STRING_CMD_CONFIG_NO_NAME    "Especifique el nombre de una variable de configuración a modificar."
#define STRING_CMD_HCONFIG_USAGE     "Lista todas las variables de configuración soportadas, a fines de completado"
#define STRING_CMD_CUSTOM_MISMATCH   "Hay diferente número de columnas y etiquetas para el informe '{1}'."
#define STRING_CMD_CUSTOM_OLD_SORT   "Empleado campo de ordenación en desuso '{1}'. Por favor modifique esto a '{2}'."
#define STRING_CMD_CUSTOM_OLD_FIELD  "Empleado campo de informe en desuso '{1}'. Por favor modifique esto a '{2}'."
#define STRING_CMD_CUSTOM_SHOWN      "{1} mostrada(s)"
#define STRING_CMD_CUSTOM_COUNT      "1 tarea"
#define STRING_CMD_CUSTOM_COUNTN     "{1} tareas"
#define STRING_CMD_CUSTOM_TRUNCATED  "truncado a {1} líneas"
#define STRING_CMD_TIMESHEET_USAGE   "Resumen semanal de tareas completadas y comenzadas"
#define STRING_CMD_TIMESHEET_STARTED "Comenzadas ({1} tareas)"
#define STRING_CMD_TIMESHEET_DONE    "Completadas ({1} tareas)"
#define STRING_CMD_MERGE_USAGE       "Combina los archivos remotos con los locales"
#define STRING_CMD_MERGE_COMPLETE    "Combinación completa."
#define STRING_CMD_MERGE_CONFIRM     "¿Quiere propagar los cambios combinados a '{1}'?"
#define STRING_CMD_MERGE_NO_URI      "No se especificó un uri para el combinado. Especifique el uri de un directorio .task remoto, o cree una entrada 'merge.default.uri' en su archivo .taskrc ."
#define STRING_CMD_BURN_USAGE_M      "Muestra un diagrama burndown, por meses"
#define STRING_CMD_BURN_USAGE_W      "Muestra un diagrama burndown, por semanas"
#define STRING_CMD_BURN_USAGE_D      "Muestra un diagrama burndown, por días"
#define STRING_CMD_BURN_TITLE        "Burndown"      // Sic
#define STRING_CMD_BURN_TOO_SMALL    "Ventana de terminal demasiado pequeña para dibujar una gráfica."
#define STRING_CMD_BURN_DAILY        "Diario"
#define STRING_CMD_BURN_WEEKLY       "Semanal"
#define STRING_CMD_BURN_MONTHLY      "Mensual"
#define STRING_CMD_BURN_STARTED      "Comenz."             // Must be 7 or fewer characters
#define STRING_CMD_BURN_DONE         "Hechas"              // Must be 7 or fewer characters
#define STRING_CMD_BURN_PENDING      "Pend."               // Must be 7 or fewer characters
#define STRING_CMD_BURN_NO_CONVERGE  "No converge"
#define STRING_CMD_HELP_USAGE        "Muestra este texto de ayuda al uso"
#define STRING_CMD_HELP_USAGE_LABEL  "Uso:"
#define STRING_CMD_HELP_USAGE_DESC   "Lanza rc.default.command, si está especificado."
#define STRING_CMD_HELP_ALIASED      "Es alias de '{1}'"
#define STRING_CMD_CAL_USAGE         "Muestra un calendario con las tareas fechadas resaltadas"
#define STRING_CMD_CAL_BAD_MONTH     "El argumento '{1}' no es un mes válido."
#define STRING_CMD_CAL_BAD_ARG       "No se pudo reconocer el argumento '{1}'."
#define STRING_CMD_CAL_LABEL_DATE    "Fecha"
#define STRING_CMD_CAL_LABEL_HOL     "Festivo"
#define STRING_CMD_CAL_SUN_MON       "La variable de configuración 'weekstart' solamente puede contener 'domingo' o 'lunes'."
#define STRING_CMD_EDIT_USAGE        "Lanza un editor para modificar una tarea directamente"

// Config
#define STRING_CONFIG_OVERNEST       "Archivo de configuración anidado hasta más de 10 niveles de profundidad - esto tiene que ser un error."
#define STRING_CONFIG_READ_INCLUDE   "No se pudo leer el archivo include '{1}'."
#define STRING_CONFIG_INCLUDE_PATH   "Solo se pueden incluir archivos con paths absolutos, no '{1}'"
#define STRING_CONFIG_BAD_ENTRY      "Formato incorrecto en entrada '{1}' en archivo de configuración."
#define STRING_CONFIG_BAD_WRITE      "No se pudo escribir a '{1}'."
#define STRING_CONFIG_DEPRECATED_US  "Su archivo .taskrc contiene ajustes de color que emplean subrayados, en desuso. Por favor, consulte:"
#define STRING_CONFIG_DEPRECATED_COL "Su archivo .taskrc contiene informes con columnas en desuso. Por favor, consulte entry_time, start_time o end_time en:"
#define STRING_CONFIG_DEPRECATED_VAR "Su archivo .taskrc contiene variables que están en desuso:"

// Context
#define STRING_CONTEXT_CREATE_RC     "No se pudo encontrar un archivo de configuración en {1}\n\n¿Desearía que se creara un {2} de ejemplo, para que taskwarrior pueda continuar?"
#define STRING_CONTEXT_NEED_RC       "No se puede continuar sin archivo de configuración (rc)."
#define STRING_CONTEXT_RC_OVERRIDE   "TASKRC sobreescrito: {1}"
#define STRING_CONTEXT_DATA_OVERRIDE "TASKDATA sobreescrito: {1}"
#define STRING_CONTEXT_SHADOW_P      "La variable de configuración 'shadow.file' está ajustada a " "sobreescribrir sus tareas pendientes. Por favor, cambie esto."
#define STRING_CONTEXT_SHADOW_C      "La variable de configuración 'shadow.file' está ajustada a " "sobreescribrir sus tareas completadas. Por favor, cambie esto."
#define STRING_CONTEXT_SHADOW_U      "La variable de configuración 'shadow.file' está ajustada a " "sobreescribrir su registro de deshacer. Por favor, cambie esto."
#define STRING_CONTEXT_SHADOW_B      "La variable de configuración 'shadow.file' está ajustada a " "sobreescribrir su archivo backlog. Por favor, cambie esto."
#define STRING_CONTEXT_SHADOW_UPDATE "[Archivo shadow '{1}' actualizado.]"

// Date
#define STRING_DATE_INVALID_FORMAT   "'{1}' no es una fecha válida según el formato '{2}'."
#define STRING_DATE_BAD_WEEKSTART    "La variable de configuración 'weekstart' solamente puede contener 'Sunday' (domingo) o 'Monday' (lunes)."
#define STRING_DATE_TOO_MUCH         "La fecha está demasiado alejada en el futuro."

#define STRING_DATE_JANUARY_LONG     "enero"
#define STRING_DATE_FEBRUARY_LONG    "febrero"
#define STRING_DATE_MARCH_LONG       "marzo"
#define STRING_DATE_APRIL_LONG       "abril"
#define STRING_DATE_MAY_LONG         "mayo"
#define STRING_DATE_JUNE_LONG        "junio"
#define STRING_DATE_JULY_LONG        "julio"
#define STRING_DATE_AUGUST_LONG      "agosto"
#define STRING_DATE_SEPTEMBER_LONG   "septiembre"
#define STRING_DATE_OCTOBER_LONG     "octubre"
#define STRING_DATE_NOVEMBER_LONG    "noviembre"
#define STRING_DATE_DECEMBER_LONG    "diciembre"

#define STRING_DATE_JANUARY_SHORT    "ene"
#define STRING_DATE_FEBRUARY_SHORT   "feb"
#define STRING_DATE_MARCH_SHORT      "mar"
#define STRING_DATE_APRIL_SHORT      "abr"
#define STRING_DATE_MAY_SHORT        "may"
#define STRING_DATE_JUNE_SHORT       "jun"
#define STRING_DATE_JULY_SHORT       "jul"
#define STRING_DATE_AUGUST_SHORT     "ago"
#define STRING_DATE_SEPTEMBER_SHORT  "sep"
#define STRING_DATE_OCTOBER_SHORT    "oct"
#define STRING_DATE_NOVEMBER_SHORT   "nov"
#define STRING_DATE_DECEMBER_SHORT   "dic"

#define STRING_DATE_SUNDAY_LONG      "domingo"
#define STRING_DATE_MONDAY_LONG      "lunes"
#define STRING_DATE_TUESDAY_LONG     "martes"
#define STRING_DATE_WEDNESDAY_LONG   "miércoles"
#define STRING_DATE_THURSDAY_LONG    "jueves"
#define STRING_DATE_FRIDAY_LONG      "viernes"
#define STRING_DATE_SATURDAY_LONG    "sábado"

#define STRING_DATE_SUNDAY_SHORT     "dom"
#define STRING_DATE_MONDAY_SHORT     "lun"
#define STRING_DATE_TUESDAY_SHORT    "mar"
#define STRING_DATE_WEDNESDAY_SHORT  "mie"
#define STRING_DATE_THURSDAY_SHORT   "jue"
#define STRING_DATE_FRIDAY_SHORT     "vie"
#define STRING_DATE_SATURDAY_SHORT   "sab"

// dependency
#define STRING_DEPEND_BLOCKED        "La tarea {1} está bloqueada por:"
#define STRING_DEPEND_BLOCKING       "y está bloqueando:"
#define STRING_DEPEND_FIX_CHAIN      "¿Desea que la cadena de dependencias sea arreglada?"

// DOM
#define STRING_DOM_UNKNOWN           "<desconocido>"
#define STRING_DOM_UNREC             "DOM: No se pudo obtener el nombre no reconocido '{1}'."
#define STRING_DOM_CANNOT_SET        "DOM: No se pudo establecer '{1}'."

// Duration
#define STRING_DURATION_UNRECOGNIZED "La duración '{1}' no se reconoció como válida, con unidades correctas como '3days'."

// E9
#define STRING_E9_UNSUPPORTED        "Operador no soportado '{1}'."
#define STRING_E9_NO_OPERANDS        "No hay operandos para el operador '{1}'."
#define STRING_E9_INSUFFICIENT_OP    "No hay suficientes operandos para el operador '{1}'."
#define STRING_E9_MORE_OP            "Encontrados operandos de más."

// edit
#define STRING_EDIT_NO_CHANGES       "No se detectaron modificaciones."
#define STRING_EDIT_NO_EDITS         "No se realizó edición."
#define STRING_EDIT_COMPLETE         "Edición completada."
#define STRING_EDIT_LAUNCHING        "Lanzando '{1}' ahora..."
#define STRING_EDIT_CHANGES          "Se detectaron modificaciones."
#define STRING_EDIT_UNPARSEABLE      "Taskwarrior no pudo manejar sus modificaciones. ¿Quiere intentarlo de nuevo?"
#define STRING_EDIT_UNWRITABLE       "No tiene permiso de escritura en su directorio data.location ."
#define STRING_EDIT_TAG_SEP          "Separe las marcas mediante espacios, así: marca1 marca2"
#define STRING_EDIT_DEP_SEP          "Las dependencias deben ser una lista separada por comas de IDs/UUIDs de tareas o intervalos de ID, sin espacios."
#define STRING_EDIT_UDA_SEP          "Atributos Definidos por el Usuario"
#define STRING_EDIT_UDA_ORPHAN_SEP   "Huérfanos de Atributo Definido por el Usuario"
#define STRING_EDIT_END              "Fin"

#define STRING_EDIT_PROJECT_MOD      "Proyecto modificado."
#define STRING_EDIT_PROJECT_DEL      "Proyecto eliminado."
#define STRING_EDIT_PRIORITY_MOD     "Prioridad modificada."
#define STRING_EDIT_PRIORITY_DEL     "Prioridad eliminada."
#define STRING_EDIT_DESC_MOD         "Descripción modificada."
#define STRING_EDIT_DESC_REMOVE_ERR  "No se puede eliminar la descripción."
#define STRING_EDIT_ENTRY_REMOVE_ERR "No se puede eliminar la fecha de creación."
#define STRING_EDIT_ENTRY_MOD        "Fecha de creación modificada."
#define STRING_EDIT_START_MOD        "Fecha de comienzo modificada."
#define STRING_EDIT_START_DEL        "Fecha de comienzo eliminada."
#define STRING_EDIT_END_MOD          "Fecha de terminación modificada."
#define STRING_EDIT_END_DEL          "Fecha de terminación eliminada."
#define STRING_EDIT_END_SET_ERR      "No se puede establecer una fecha de completado en una tarea pendiente."
#define STRING_EDIT_SCHED_MOD        "Fecha programada modificada."
#define STRING_EDIT_SCHED_DEL        "Fecha programada eliminada."
#define STRING_EDIT_DUE_MOD          "Fecha de vencimiento modificada."
#define STRING_EDIT_DUE_DEL          "Fecha de vencimiento eliminada."
#define STRING_EDIT_DUE_DEL_ERR      "No se puede eliminar una fecha de vencimiento de una tarea recurrente."
#define STRING_EDIT_UNTIL_MOD        "Fecha 'hasta' modificada."
#define STRING_EDIT_UNTIL_DEL        "Fecha 'hasta' eliminada."
#define STRING_EDIT_RECUR_MOD        "Recurrencia modificada."
#define STRING_EDIT_RECUR_DEL        "Recurrencia eliminada."
#define STRING_EDIT_RECUR_DUE_ERR    "Una tarea recurrente debe tener una fecha de vencimiento."
#define STRING_EDIT_RECUR_ERR        "No es una duración de recurrencia válida."
#define STRING_EDIT_WAIT_MOD         "Fecha de espera modificada."
#define STRING_EDIT_WAIT_DEL         "Fecha de espera eliminada."
#define STRING_EDIT_PARENT_MOD       "UUID primario modificado."
#define STRING_EDIT_PARENT_DEL       "UUID primario eliminado."
#define STRING_EDIT_UDA_MOD          "UDA {1} modificado."
#define STRING_EDIT_UDA_DEL          "UDA {1} eliminado."

// These four blocks can be replaced, but the number of lines must not change.
#define STRING_EDIT_HEADER_1         "El comando 'task <id> edit' le permite modificar todos los detalles de"
#define STRING_EDIT_HEADER_2         "una tarea usando un editor de textos. Más abajo hay una representación"
#define STRING_EDIT_HEADER_3         "de todos los detalles de la tarea. Modifique lo que desee y, cuando grabe"
#define STRING_EDIT_HEADER_4         "y salga del editor, taskwarrior leerá este archivo, determinará qué ha"
#define STRING_EDIT_HEADER_5         "cambiado, y aplicará esos cambios. Si sale del editor sin grabar o sin"
#define STRING_EDIT_HEADER_6         "hacer modificaciones, taskwarrior no hará nada."

#define STRING_EDIT_HEADER_7         "Las líneas que comienzan con # representan datos que no puede cambiar, como ID."
#define STRING_EDIT_HEADER_8         "Si es demasiado creativo con sus modificaciones, taskwarrior le enviará de vuelta"
#define STRING_EDIT_HEADER_9         "al editor para que lo intente de nuevo."

#define STRING_EDIT_HEADER_10        "Si queda atrapado en un ciclo sin fin, volviendo a editar el mismo archivo"
#define STRING_EDIT_HEADER_11        "una y otra vez, simplemente abandone el editor sin hacer ningún cambio."
#define STRING_EDIT_HEADER_12        "Taskwarrior lo advertirá y detendrá el proceso."

#define STRING_EDIT_HEADER_13        "Las anotaciones aparecen así: <date> -- <text> y puede haber cualquier número de ellas."
#define STRING_EDIT_HEADER_14        "El separador ' -- ' entre la fecha y el campo de texto no debe ser eliminado."
#define STRING_EDIT_HEADER_15        "La última es una \"entrada en blanco\" para añadir una anotación, si le conviene:"

// Maintain the same spacing.
#define STRING_EDIT_TABLE_HEADER_1   "Nombre             Detalles modificables"
#define STRING_EDIT_TABLE_HEADER_2   "-----------------  ----------------------------------------------------"

// Errors
// TODO Move each of these to appropriate section.
#define STRING_ERROR_PREFIX          "Error: "
#define STRING_UNKNOWN_ERROR         "Error desconocido."
#define STRING_NO_HOME               "No se pudo leer el directorio personal en el archivo passwd."
#define STRING_TRIVIAL_INPUT         "Debe especificar un comando o una tarea a modificar."
#define STRING_ASSUME_INFO           "No especificó un comando - asumido 'information'."
#define STRING_INFINITE_LOOP         "Sustitución terminada debido a que se hicieron más de {1} cambios - protección contra ciclo eterno."
#define STRING_UDA_TYPE              "Los atributos definidos por el usuario solamente pueden ser de los tipos 'string', 'date', 'duration' o 'numeric'."
#define STRING_UDA_TYPE_MISSING      "No se encontró uda.{1}.type . El UDA '{1}' debe tener algún tipo especificado."
#define STRING_UDA_NUMERIC           "El valor '{1}' no es un valor numérico válido."
#define STRING_UDA_COLLISION         "El UDA denominado '{1}' es el mismo que un atributo del núcleo, y no está permitido."
#define STRING_INVALID_MOD           "El atributo '{1}' no admite un valor '{2}'."
#define STRING_INVALID_SORT_COL      "La columna '{1}' no es un campo de ordenación válido."
#define STRING_TLS_INIT_FAIL         "Error initializing TLS."

// Feedback
#define STRING_FEEDBACK_NO_TASKS     "Ninguna tarea."
#define STRING_FEEDBACK_NO_TASKS_SP  "No se especificaron tareas."
#define STRING_FEEDBACK_NO_MATCH     "Ninguna coincidencia."
#define STRING_FEEDBACK_TASKS_SINGLE "(1 tarea)"
#define STRING_FEEDBACK_TASKS_PLURAL "({1} tareas)"
#define STRING_FEEDBACK_DELETED      "Se eliminará {1}."
#define STRING_FEEDBACK_DEP_SET      "Las dependencias se ajustarán a '{1}'."
#define STRING_FEEDBACK_DEP_MOD      "Las dependencias se cambiarán de '{1}' a '{2}'."
#define STRING_FEEDBACK_DEP_DEL      "Dependencias '{1}' eliminadas."
#define STRING_FEEDBACK_DEP_WAS_SET  "Dependencias ajustadas a '{1}'."
#define STRING_FEEDBACK_DEP_WAS_MOD  "Dependencias cambiadas de '{1}' a '{2}'."
#define STRING_FEEDBACK_ATT_SET      "{1} se establecerá como '{2}'."
#define STRING_FEEDBACK_ATT_MOD      "{1} se cambiará de '{2}' a '{3}'."
#define STRING_FEEDBACK_ATT_DEL      "{1} eliminado."
#define STRING_FEEDBACK_ATT_DEL_DUR  "{1} eliminado (duración: {2})."
#define STRING_FEEDBACK_ATT_WAS_SET  "{1} establecido como '{2}'."
#define STRING_FEEDBACK_ATT_WAS_MOD  "{1} cambiado de '{2}' a '{3}'."
#define STRING_FEEDBACK_ANN_ADD      "Anotación de '{1}' añadida."
#define STRING_FEEDBACK_ANN_DEL      "Anotación '{1}' eliminada."
#define STRING_FEEDBACK_ANN_WAS_MOD  "Anotación cambiada a '{1}'."
#define STRING_FEEDBACK_NOP          "No se harán cambios."
#define STRING_FEEDBACK_WAS_NOP      "No se hicieron cambios."
#define STRING_FEEDBACK_TAG_NOCOLOR  "La marca especial 'nocolor' deshabilitará las reglas de color para esta tarea."
#define STRING_FEEDBACK_TAG_NONAG    "La marca especial 'nonag' evitará el recuerdo fastidioso cuando la tarea sea modificada."
#define STRING_FEEDBACK_TAG_NOCAL    "La marca especial 'nocal' mantendrá esta tarea fuera del informe 'calendar'."
#define STRING_FEEDBACK_TAG_NEXT     "La etiqueta especial 'next' aumentará la urgencia de esta tarea para que aparezca en el informe 'next'."
#define STRING_FEEDBACK_UNBLOCKED    "Desbloqueada {1} '{2}'."
#define STRING_FEEDBACK_EXPIRED      "La tarea {1} '{2}' caducó y fue eliminada."

// File
#define STRING_FILE_PERMS            "Taskwarrior no tiene los permisos adecuados para '{1}'."

// helpers
#define STRING_HELPER_PROJECT_CHANGE "El proyecto '{1}' ha cambiado."
#define STRING_HELPER_PROJECT_COMPL  "El proyecto '{1}' se ha completado en un {2}%"
#define STRING_HELPER_PROJECT_REM    "(quedan {1} de {2} tareas)."

// interactive
#define STRING_INTERACTIVE_WIDTH     "Context::getWidth: anchura determinada de {1} caracteres"
#define STRING_INTERACTIVE_HEIGHT    "Context::getHeight: altura determinada de {1} caracteres"

// JSON
#define STRING_JSON_MISSING_VALUE    "Error: falta valor después de ',' en posición {1}"
#define STRING_JSON_MISSING_VALUE2   "Error: falta valor en posición {1}"
#define STRING_JSON_MISSING_BRACKET  "Error: falta ']' en posición {1}"
#define STRING_JSON_MISSING_BRACE    "Error: falta '}' en posición {1}"
#define STRING_JSON_MISSING_COLON    "Error: falta ':' en posición {1}"
#define STRING_JSON_MISSING_OPEN     "Error: se esperaba '{' or '[' en posición {1}"
#define STRING_JSON_EXTRA_CHARACTERS "Error: encontrados caracteres extra en posición {1}"

// Legacy
#define STRING_LEGACY_FEATURE        "Note: la función '{1}' está en desuso."

// Record
#define STRING_RECORD_EMPTY          "Registro vacío en la entrada."
#define STRING_RECORD_JUNK_AT_EOL    "Caracteres no reconocidos al final de línea."
#define STRING_RECORD_NOT_FF4        "Registro no reconocido como formato 4."

// 'show' command
#define STRING_CMD_SHOW              "Muestra todas las variables de configuración o un subconjunto"
#define STRING_CMD_SHOW_ARGS         "Solo puede especificar 'all' o un término de búsqueda."
#define STRING_CMD_SHOW_NONE         "No hay variables de configuración que coincidan."
#define STRING_CMD_SHOW_UNREC        "Su archivo .taskrc contiene estas variables no reconocidas:"
#define STRING_CMD_SHOW_DIFFER       "Algunas de sus variables .taskrc difieren de los valores por defecto."
#define STRING_CMD_SHOW_HOOKS        "Su archivo .taskrc contiene estos hook scripts que faltan o no se pueden leer:"
#define STRING_CMD_SHOW_EMPTY        "Error de configuración: .taskrc no contiene ninguna entrada."
#define STRING_CMD_SHOW_DIFFER_COLOR "Las cambiadas aparecen resaltadas en {1} más arriba."
#define STRING_CMD_SHOW_CONFIG_ERROR "Error de configuración: {1} contiene un valor '{2}' no reconocido."
#define STRING_CMD_SHOW_NO_LOCATION  "Error de configuración: data.location no especificado en el archivo .taskrc ."
#define STRING_CMD_SHOW_LOC_EXIST    "Error de configuración: data.location contiene un nombre de directorio que no existe o no se puede leer."
#define STRING_CMD_SHOW_CONF_VAR     "Variable Config"
#define STRING_CMD_SHOW_CONF_VALUE   "Valor"
#define STRING_CMD_SHOWRAW           "Muestra todas las variables de configuración en un formato legible por la máquina"

// Task
#define STRING_TASK_NO_FF1           "Taskwarrior ya no admite el formato de archivo 1, usado originalmente entre el 27 de noviembre del 2006 y el 31 de diciembre del 2007."
#define STRING_TASK_NO_FF2           "Taskwarrior no longer supports file format 2, originally used between 1 January 2008 and 12 April 2009."
#define STRING_TASK_PARSE_ANNO_BRACK "Faltan corchetes de anotación."
#define STRING_TASK_PARSE_ATT_BRACK  "Faltan corchetes de atributo."
#define STRING_TASK_PARSE_TAG_BRACK  "Faltan corchetes de marca."
#define STRING_TASK_PARSE_TOO_SHORT  "Línea demasiado corta."
#define STRING_TASK_PARSE_UNREC_FF   "Formato de archivo taskwarrior no reconocido."
#define STRING_TASK_DEPEND_ITSELF    "Una tarea no puede depender de sí misma."
#define STRING_TASK_DEPEND_MISS_CREA "No se pudo crear una dependencia de la tarea {1} - no encontrada."
#define STRING_TASK_DEPEND_MISS_DEL  "No se pudo eliminar una dependencia de la tarea {1} - no encontrada."
#define STRING_TASK_DEPEND_DUP       "La tarea {1} ya depende de la tarea {2}."
#define STRING_TASK_DEPEND_CIRCULAR  "Dependencia circular detectada y anulada."
#define STRING_TASK_VALID_DESC       "Una tarea debe tener descripción."
#define STRING_TASK_VALID_BLANK      "No se puede añadir una tarea que está en blanco."
#define STRING_TASK_VALID_BEFORE     "Advertencia: ha especificado que la fecha '{1}' es después de la fecha '{2}'."
#define STRING_TASK_VALID_REC_DUE    "Una tarea recurrente debe tener también una fecha de vencimiento."
#define STRING_TASK_VALID_RECUR      "El valor de recurrencia '{1}' no es válido."
#define STRING_TASK_VALID_PRIORITY   "Valores de prioridad pueden ser 'H', 'M' o 'L', no '{1}'."
#define STRING_TASK_SAFETY_VALVE     "Este comando no tiene filtro, y modificará todas las tareas. ¿Está seguro?"
#define STRING_TASK_SAFETY_FAIL      "Se impidió la ejecución del comando."

// Taskmod
#define STRING_TASKMOD_BAD_INIT      "Taskmod::getUuid(): objeto Task no inicializado."
#define STRING_TASKMOD_TIME          "tiempo "
#define STRING_TASKMOD_OLD           "antiguo "
#define STRING_TASKMOD_NEW           "nuevo "

// TDB2
#define STRING_TDB2_PARSE_ERROR      " en {1} en la línea {2}"
#define STRING_TDB2_UUID_NOT_UNIQUE  "No se puede añadir la tarea porque el uuid '{1}' no es único."

#define STRING_TDB2_UNDO_TIMESTAMP   "Hubo un problema al leer la marca de tiempo del archivo undo.data ."
#define STRING_TDB2_UNREADABLE       "No se puede leer '{1}'."
#define STRING_TDB2_UNWRITABLE       "No se puede escribir '{1}'."
#define STRING_TDB2_NO_CHANGES       "No hay cambios para combinar."
#define STRING_TDB2_REMOTE_CHANGE    "Encontrado cambio remoto en   {1}  \"{2}\""
#define STRING_TDB2_LOCAL_CHANGE     "Reteniendo cambios locales a  {1}  \"{2}\""
#define STRING_TDB2_MISSING          "Falta                         {1}  \"{2}\""
#define STRING_TDB2_MERGING          "Combinando tarea remota nueva {1}  \"{2}\""
#define STRING_TDB2_UP_TO_DATE       "La base de datos está actualizada, no se requiere combinación."
#define STRING_TDB2_NO_UNDO          "No hay transacciones registradas para deshacer."
#define STRING_TDB2_LAST_MOD         "La última modificación fue hecha {1}"
#define STRING_TDB2_UNDO_PRIOR       "Valores Previos"
#define STRING_TDB2_UNDO_CURRENT     "Valores Actuales"
#define STRING_TDB2_DIFF_PREV        "--- estado previo "             // Same length
#define STRING_TDB2_DIFF_PREV_DESC   "Deshacer restaurará este estado"//   ||
#define STRING_TDB2_DIFF_CURR        "+++ estado actual "             // Same length
#define STRING_TDB2_DIFF_CURR_DESC   "Cambio hecho {1}"
#define STRING_TDB2_UNDO_CONFIRM     "El comando undo es irreversible. ¿Está seguro de querer revertir al estado anterior?"
#define STRING_TDB2_MISSING_UUID     "No se pudo localizar el UUID en la tarea a deshacer."
#define STRING_TDB2_REVERTED         "Tarea modificada revertida."
#define STRING_TDB2_REMOVED          "Tarea eliminada."
#define STRING_TDB2_UNDO_COMPLETE    "Deshacer completado."
#define STRING_TDB2_MISSING_TASK     "Tarea con UUID {1} no encontrada en los datos."
#define STRING_TDB2_UNDO_IMPOSSIBLE  "No es posible deshacer."

// text
                                     // Se añade al final una lista de comandos separados por comas.
                                     // A comma-separated list of commands is appended.
#define STRING_TEXT_AMBIGUOUS        "Ambiguo {1} '{2}' - puede ser "

// Transport
#define STRING_TRANSPORT_NORUN       "No se pudo lanzar '{1}'. ¿Está instalado y disponible en $PATH?"
#define STRING_TRANSPORT_NOFORK      "No se pudo ejecutar '{1}': {2}. ¿Puede ser que su sistema esté sin recursos?"

#define STRING_TRANSPORT_URI_NODIR   "El uri '{1}' no parece ser un directorio."
#define STRING_TRANSPORT_CURL_URI    "Cuando se usa el protocolo 'curl' el uri debe contener un nombre de máquina."
#define STRING_TRANSPORT_CURL_WILDCD "Cuando se usa el protocolo 'curl' no están soportados los comodines."
#define STRING_TRANSPORT_CURL_FAIL   "Curl falló, consulte los mensajes precedentes."
#define STRING_TRANSPORT_RSYNC_URI   "Cuando se usa el protocolo 'rsync' el uri debe contener un nombre de máquina."
#define STRING_TRANSPORT_RSYNC_FAIL  "Rsync falló, consulte los mensajes precedentes."
#define STRING_TRANSPORT_SSH_URI     "Cuando se usa el protocolo 'ssh' el uri debe contener un nombre de máquina."
#define STRING_TRANSPORT_SSH_FAIL    "Ssh falló, consulte los mensajes precedentes."
#define STRING_TRANSPORT_SHELL_NOPATH "Cuando se utiliza el protocolo 'sh + cp' para copiar varios archivos se debe especificar un path."
#define STRING_TRANSPORT_SHELL_FAIL  "Fallo de comando del entorno, consulte la salida anterior."

// Uri
#define STRING_URI_QUOTES            "No se pudo interpretar el uri '{1}', uso erróneo de comillas simples."
#define STRING_URI_BAD_FORMAT        "El uri '{1}' no está en el formato esperado."

// utf8
#define STRING_UTF8_INVALID_CP_REP   "Representación de codepoint no válida."
#define STRING_UTF8_INVALID_CP       "Codepoint Unicode no válido."

// View
#define STRING_VIEW_TOO_SMALL        "El informe tiene una anchura mínima de {1} y no cabe en la disponible, que es {2}."

// ****************************************************************************************************
// Usage text.  This is an exception, and contains \n characters and formatting.
/* FIXME más abajo 
  "  task project:Casa due.before:hoy          <command> <mods>\n" 
  "  task project:Home due.before:today        <command> <mods>\n" 
 y 
  "  task <filter> <command> project:Casa\n" 
  "  task <filter> <command> +finde +jardín due:mañana\n" 
  "  task <filter> <command> Texto anotación/descripción\n" 
 en vez de: 
  "  task <filter> <command> project:Home\n" 
  "  task <filter> <command> +weekend +garden due:tomorrow\n" 
  "  task <filter> <command> Description/annotation text\n" 
 y 
  "  task add -- project:Casa necesita programación\n" 
  "  task add -- project:Home needs scheduling\n" */
 #define STRING_CMD_HELP_TEXT \
  "La documentación de Taskwarrior se puede consultar usando 'man task', 'man taskrc'" \
  "man task-tutorial', 'man task-color', 'man task-faq', 'man task-synch o en " \
  "http://taskwarrior.org\n" \
  "\n" \
  "La forma general para los comandos es:\n" \
  "  task [<filter>] <command> [<mods>]\n" \
  "\n" \
  "<filter> consiste en cero o más restricciones sobre qué tareas seleccionar, " \
  "como en:\n" \
  "  task                                      <command> <mods>\n" \
  "  task 28                                   <command> <mods>\n" \
  "  task +weekend                             <command> <mods>\n" \
  "  task project:Home due.before:today        <command> <mods>\n" \
  "  task ebeeab00-ccf8-464b-8b58-f7f2d606edfb <command> <mods>\n" \
  "\n" \
  "Por defecto, los elementos filtro son combinados con un operador 'and' implícito, pero " \
  "'or' y 'xor' también se pueden usar, siempre que se incluyan paréntesis:\n" \
  "  task '(/[Cc]at|[Dd]og/ or /[0-9]+/)'      <command> <mods>\n" \
  "\n" \
  "Un filtro puede dirigirse a tareas específicas mediante números ID o UUID. Para especificar " \
  "múltiples tareas use una de estas formas:\n" \
  "  task 1,2,3                                    delete\n" \
  "  task 1-3                                      info\n" \
  "  task 1,2-5,19                                 modify pri:H\n" \
  "  task 4-7 ebeeab00-ccf8-464b-8b58-f7f2d606edfb info\n" \
  "\n" \
  "<mods> consiste en cero o más cambios para aplicar a las tareas seleccionadas, " \
  "como en:\n" \
  "  task <filter> <command> project:Home\n" \
  "  task <filter> <command> +weekend +garden due:tomorrow\n" \
  "  task <filter> <command> Description/annotation text\n" \
  "\n" \
  "Las marcas son palabras arbitrarias, en cualquier número:\n" \
  "  +marca     El + significa añade la marca\n" \
  "  -marca     El - significa quita la marca\n" \
  "\n" \
  "Son atributos internos:\n" \
  "  description:    Texto descriptivo de la tarea\n" \
  "  status:         Estatus de la tarea - pendiente, completada, eliminada, esperando\n" \
  "  project:        Nombre del proyecto\n" \
  "  priority:       Prioridad\n" \
  "  due:            Fecha de vencimiento\n" \
  "  recur:          Frecuencia de recurrencia\n" \
  "  until:          Fecha de caducidad de la tarea\n" \
  "  limit:          Número de filas deseado en un informe, o 'page'\n" \
  "  wait:           Fecha en la que la tarea se transforma en pendiente\n" \
  "  entry:          Fecha en la que la tarea fue creada\n" \
  "  end:            Fecha en la que la tarea fue completada/eliminada\n" \
  "  start:          Fecha en la que la tarea fue comenzada\n" \
  "  scheduled:      Fecha programada para que la tarea comience\n" \
  "  depends:        Otras tareas de las que esta tarea depende\n" \
  "\n" \
  "Los modificadores de atributos hacen a los filtros más precisos. Modificadores soportados son:\n" \
  "  before     (sinónimos under, below)\n" \
  "  after      (sinónimos over, above)\n" \
  "  none\n" \
  "  any\n" \
  "  is         (sinónimo equals)\n" \
  "  isnt       (sinónimo not)\n" \
  "  has        (sinónimo contains)\n" \
  "  hasnt\n" \
  "  startswith (sinónimo left)\n" \
  "  endswith   (sinónimo right)\n" \
  "  word\n" \
  "  noword\n" \
  "\n" \
  "Soporte de expresiones algebraicas alternativas:\n" \
  "  and  or  xor            Operadores lógicos\n" \
  "  <  <=  =  !=  >=  >     Operadores relacionales\n" \
  "  (  )                    Precedencia\n" \
  "\n" \
  "  task due.before:eom priority.not:L   list\n" \
  "  task '(due < eom or priority != L)'  list\n" \
  "\n" \
  "El archivo .taskrc por defecto puede ser sobreescrito mediante:\n" \
  "  task ... rc:<alternate file> ...\n" \
  "  task ... rc:~/.alt_taskrc ...\n" \
  "\n" \
  "Los valores en .taskrc (u otro) pueden ser sobreescritos mediante:\n" \
  "  task ... rc.<name>=<value> ...\n" \
  "  task rc.color=off list\n" \
  "\n" \
  "Cualquier nombre de comando o atributo puede ser abreviado mientras permanezca único:\n" \
  "  task list project:Home\n" \
  "  task li       pro:Home\n" \
  "\n" \
  "Algunas descripciones de tarea necesitan ser escapadas debido al entorno de comandos:\n" \
  "  task add \"quoted ' quote\"\n" \
  "  task add escaped \\' quote\n" \
  "\n" \
  "El argumento -- indica a taskwarrior que trate todos los restantes argumentos como descripción, incluso " \
  "aunque de otra manera hubieran sido argumentos o marcas:\n" \
  "  task add -- project:Home needs scheduling\n" \
  "\n" \
  "Muchos caracteres tienen significado especial para el entorno de comandos, incluyendo:\n" \
  "  $ ! ' \" ( ) ; \\ ` * ? { } [ ] < > | & % # ~\n" \
  "\n"

/*
  To be included later, before the 'precendence' line.
 
  "  +  -                    Addition, subtraction\n" \
  "  !                       Inversion\n" \
  "  ~  !~                   Match, no match\n" \
*/

// util
#define STRING_UTIL_CONFIRM_YN       " (yes/no) "
#define STRING_UTIL_CONFIRM_YES      "yes"
#define STRING_UTIL_CONFIRM_YES_U    "Yes"
#define STRING_UTIL_CONFIRM_NO       "no"
#define STRING_UTIL_CONFIRM_ALL      "all"
#define STRING_UTIL_CONFIRM_ALL_U    "All"
#define STRING_UTIL_CONFIRM_QUIT     "quit"
#define STRING_UTIL_GIBIBYTES        "GiB"
#define STRING_UTIL_MEBIBYTES        "MiB"
#define STRING_UTIL_KIBIBYTES        "KiB"
#define STRING_UTIL_BYTES            "B"

// shell
#define STRING_SHELL_USAGE \
  "Uso: tasksh [<fichero-comandos>]   Ejecuta los comandos task contenidos en <fichero-comandos>, si se indica;\n" \
  "                                   si no, lanza un entorno interactivo de comandos task.\n" \
  "     tasksh --version              Muestra la versión de task.\n" \
  "     tasksh --help                 Muestra esta ayuda.\n"

#define STRING_SHELL_NO_FILE         "El fichero de entrada no existe.\n"

#endif

