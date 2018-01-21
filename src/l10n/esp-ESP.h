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

// columns/Col*
#define STRING_COLUMN_LABEL_DESC     "Descripción"
#define STRING_COLUMN_LABEL_DUE      "Vencimiento"
#define STRING_COLUMN_LABEL_END      "Fin"
#define STRING_COLUMN_LABEL_ENTERED  "Entrada"
#define STRING_COLUMN_LABEL_COUNT    "Recuento"
#define STRING_COLUMN_LABEL_COMPLETE "Completada"
#define STRING_COLUMN_LABEL_MOD      "Modificada"
#define STRING_COLUMN_LABEL_ADDED    "Añadida"
#define STRING_COLUMN_LABEL_AGE      "Edad"
#define STRING_COLUMN_LABEL_PROJECT  "Proyecto"
#define STRING_COLUMN_LABEL_UNTIL    "Hasta"
#define STRING_COLUMN_LABEL_WAIT     "Espera"
#define STRING_COLUMN_LABEL_WAITING  "Esperando hasta"
#define STRING_COLUMN_LABEL_RECUR    "Recur"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_RECUR_L  "Recurrencia"
#define STRING_COLUMN_LABEL_START    "Comienzo"
#define STRING_COLUMN_LABEL_STARTED  "Comenzada"
#define STRING_COLUMN_LABEL_ACTIVE   "A"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_STATUS   "Estatus"
#define STRING_COLUMN_LABEL_STAT     "Est"
#define STRING_COLUMN_LABEL_STAT_PE  "Pendientes"
#define STRING_COLUMN_LABEL_STAT_CO  "Completadas"
#define STRING_COLUMN_LABEL_STAT_DE  "Suprimidas"
#define STRING_COLUMN_LABEL_STAT_WA  "Esperando"
// Mejor Periódica, pero STRING_COLUMN_LABEL_STAT_P es Pendiente
#define STRING_COLUMN_LABEL_STAT_RE  "Recurrentes"
#define STRING_COLUMN_LABEL_STAT_P   "P"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_STAT_C   "C"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_STAT_D   "S"
#define STRING_COLUMN_LABEL_STAT_W   "E"
#define STRING_COLUMN_LABEL_STAT_R   "R"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_TAGS     "Marcas"
#define STRING_COLUMN_LABEL_TAG      "Marca"
#define STRING_COLUMN_LABEL_UUID     "UUID"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_URGENCY  "Urgencia"
#define STRING_COLUMN_LABEL_NAME     "Nombre"
#define STRING_COLUMN_LABEL_VALUE    "Valor"
#define STRING_COLUMN_LABEL_DATE     "Fecha"
#define STRING_COLUMN_LABEL_COLUMN   "Columnas"
#define STRING_COLUMN_LABEL_STYLES   "Formatos soportados"
#define STRING_COLUMN_LABEL_EXAMPLES "Ejemplo"
#define STRING_COLUMN_LABEL_SCHED    "Programada"
#define STRING_COLUMN_LABEL_UDA      "Nombre"
#define STRING_COLUMN_LABEL_TYPE     "Tipo"
#define STRING_COLUMN_LABEL_MODIFY   "Modificable"
#define STRING_COLUMN_LABEL_NOMODIFY "Solo lectura"
#define STRING_COLUMN_LABEL_LABEL    "Etiqueta"
#define STRING_COLUMN_LABEL_DEFAULT  "Defecto"
#define STRING_COLUMN_LABEL_VALUES   "Valores permitidos"
#define STRING_COLUMN_LABEL_UDACOUNT "Recuento de uso"
#define STRING_COLUMN_LABEL_ORPHAN   "UDA huérfano"

#define STRING_COLUMN_LABEL_COMMAND  "Comando"
#define STRING_COLUMN_LABEL_CATEGORY "Categoría"
#define STRING_COLUMN_LABEL_RO       "R/W"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_SHOWS_ID "ID"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_GC       "GC"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_CONTEXT  "Contexto"
#define STRING_COLUMN_LABEL_FILTER   "Filtro"
#define STRING_COLUMN_LABEL_MODS     "Mods"  // |esp-ESP|==|eng-USA|
#define STRING_COLUMN_LABEL_MISC     "Misc"  // |esp-ESP|==|eng-USA|

// Column Examples
#define STRING_COLUMN_EXAMPLES_TAGS  "casa @tarea"
#define STRING_COLUMN_EXAMPLES_PROJ  "casa.jardín"
#define STRING_COLUMN_EXAMPLES_PAR   "casa"
#define STRING_COLUMN_EXAMPLES_IND   "  casa.jardín"
#define STRING_COLUMN_EXAMPLES_DESC  "Mueve tu ropa a la percha de abajo"
#define STRING_COLUMN_EXAMPLES_ANNO1 "Inmediatamente antes de comer"
#define STRING_COLUMN_EXAMPLES_ANNO2 "Si juegas en el partido de esta tarde"
#define STRING_COLUMN_EXAMPLES_ANNO3 "Antes de que escribas a casa"
#define STRING_COLUMN_EXAMPLES_ANNO4 "Si no te estás cortando el pelo"

// commands/Cmd*
#define STRING_CMD_EXEC_USAGE        "Ejecuta comandos y scripts externos"
#define STRING_CMD_URGENCY_USAGE     "Muestra la medida de urgencia de una tarea"
#define STRING_CMD_URGENCY_RESULT    "tarea {1} urgencia {2}"
#define STRING_CMD_ADD_USAGE         "Añade una nueva tarea"
#define STRING_CMD_ADD_FEEDBACK      "Creada tarea {1}."
#define STRING_CMD_ADD_RECUR         "Creada tarea {1} (modelo de recurrencia)."
#define STRING_CMD_LOG_USAGE         "Añade una nueva tarea que ya ha sido completada"
#define STRING_CMD_LOG_NO_RECUR      "No puede registrar tareas recurrentes."
#define STRING_CMD_LOG_NO_WAITING    "No puede registrar tareas en espera."
#define STRING_CMD_LOG_LOGGED        "Tarea {1} registrada."

#define STRING_CMD_IDS_USAGE_RANGE   "Muestra los IDs de las tareas coincidentes, como un rango"
#define STRING_CMD_IDS_USAGE_LIST    "Muestra los IDs de las tareas coincidentes, en forma de lista"
#define STRING_CMD_IDS_USAGE_ZSH     "Muestra los IDs y descripciones de las tareas coincidentes"
#define STRING_CMD_UDAS_USAGE        "Muestra detalles de todos los UDA definidos"
#define STRING_CMD_UDAS_COMPL_USAGE  "Muestra los UDAs definidos con fines de terminación"
#define STRING_CMD_UUIDS_USAGE_RANGE "Muestra los UUIDs de las tareas coincidentes, como una lista separada por comas"
#define STRING_CMD_UUIDS_USAGE_LIST  "Muestra los UUIDs de las tareas coincidentes, como una lista"
#define STRING_CMD_UUIDS_USAGE_ZSH   "Muestra los UUIDs y descripciones de las tareas coincidentes"
// metadatos: RAE 3ªed
#define STRING_CMD_INFO_USAGE        "Muestra todos los datos y metadatos"
#define STRING_CMD_INFO_BLOCKED      "Esta tarea está bloqueada por"
#define STRING_CMD_INFO_BLOCKING     "Esta tarea bloquea"
#define STRING_CMD_INFO_UNTIL        "Hasta"
#define STRING_CMD_INFO_MODIFICATION "Modificación"
#define STRING_CMD_INFO_MODIFIED     "Modificada por última vez"
#define STRING_CMD_INFO_VIRTUAL_TAGS "Marcas virtuales"
#define STRING_CMD_UNDO_USAGE        "Revierte el cambio más reciente a una tarea"
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

#define STRING_CMD_HISTORY_USAGE_D   "Shows a report of task history, by day"
#define STRING_CMD_HISTORY_USAGE_W   "Shows a report of task history, by week"
#define STRING_CMD_HISTORY_DAY       "Day"
#define STRING_CMD_GHISTORY_USAGE_D  "Shows a graphical report of task history, by day"
#define STRING_CMD_GHISTORY_USAGE_W  "Shows a graphical report of task history, by week"
#define STRING_CMD_GHISTORY_USAGE_D  "Shows a graphical report of task history, by day"
#define STRING_CMD_GHISTORY_USAGE_W  "Shows a graphical report of task history, by week"
#define STRING_CMD_GHISTORY_DAY      "Day"

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
#define STRING_CMD_UNIQUE_USAGE      "Genera listas de valores de atributo únicos"
#define STRING_CMD_UNIQUE_MISSING    "Se debe especificar un atributo. Ver 'task _columns'."
#define STRING_CMD_UNIQUE_VALID      "Debe especificar un atributo o un UDA."

#define STRING_CMD_IMPORT_USAGE      "Importa archivos JSON"
#define STRING_CMD_IMPORT_SUMMARY    "Importadas {1} tareas."
#define STRING_CMD_IMPORT_FILE       "Importando '{1}'"
#define STRING_CMD_IMPORT_MISSING    "Archivo '{1}' no encontrado."
#define STRING_CMD_IMPORT_UUID_BAD   "UUID '{1}' no válido."
#define STRING_TASK_NO_DESC          "La anotación carece de descripción: {1}"
#define STRING_TASK_NO_ENTRY         "La anotación carece de fecha de entrada: {1}"

#define STRING_CMD_COMMANDS_USAGE    "Genera una lista de todos los comandos, con detalles de comportamiento"
#define STRING_CMD_HCOMMANDS_USAGE   "Genera una lista de todos los comandos, con fines de auto-completado"
#define STRING_CMD_ZSHCOMMANDS_USAGE "Genera una lista de todos los comandos, con fines de auto-completado zsh"

#define STRING_CMD_ZSHATTS_USAGE     "Genera una lista de todos los atributos, con fines de auto-completado zsh"
#define STRING_CMD_ALIASES_USAGE     "Genera una lista de todos los alias, con fines de auto-completado"

#define STRING_CMD_CUSTOM_MISMATCH   "Hay diferente número de columnas y etiquetas para el informe '{1}'."
#define STRING_CMD_CUSTOM_SHOWN      "{1} mostrada(s)"
#define STRING_CMD_CUSTOM_COUNT      "1 tarea"
#define STRING_CMD_CUSTOM_COUNTN     "{1} tareas"
#define STRING_CMD_CUSTOM_TRUNCATED  "truncado a {1} líneas"
#define STRING_CMD_CAL_USAGE         "Muestra un calendario con las tareas fechadas resaltadas"
#define STRING_CMD_CAL_BAD_MONTH     "El argumento '{1}' no es un mes válido."
#define STRING_CMD_CAL_BAD_ARG       "No se pudo reconocer el argumento '{1}'."
#define STRING_CMD_CAL_LABEL_DATE    "Fecha"
#define STRING_CMD_CAL_LABEL_HOL     "Festivo"
#define STRING_CMD_CAL_SUN_MON       "La variable de configuración 'weekstart' solamente puede contener 'domingo' o 'lunes'."
#define STRING_CMD_CALC_USAGE        "Calculadora"

// Feedback
#define STRING_FEEDBACK_NO_TASKS     "Ninguna tarea."
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
#define STRING_FEEDBACK_TAG_VIRTUAL  "Las marcas virtuales (incluída '{1}') están reservadas y no pueden ser añadidas o eliminadas."
#define STRING_FEEDBACK_UNBLOCKED    "Desbloqueada {1} '{2}'."
#define STRING_FEEDBACK_EXPIRED      "La tarea {1} '{2}' caducó y fue eliminada."
#define STRING_FEEDBACK_BACKLOG_N    "Hay {1} modificaciones locales.  Se require una sincronización."
#define STRING_FEEDBACK_BACKLOG      "Hay {1} modificaciones locales.  Se require una sincronización."

// Task
#define STRING_TASK_NO_FF1           "Taskwarrior ya no admite el formato de archivo 1, usado originalmente entre el 27 de noviembre de 2006 y el 31 de diciembre de 2007."
#define STRING_TASK_NO_FF2           "Taskwarrior ya no admite el formato de archivo 2, usado originalmente entre el 1 de enero de 2008 y el 12 de abril de 2009."
#define STRING_TASK_NO_FF3           "Taskwarrior ya no admite el formato de archivo 3, usado originalmente entre el 23 de marzo de 2009 y el  16 de mayo de 2009."
#define STRING_TASK_PARSE_UNREC_FF   "Formato de archivo taskwarrior no reconocido."
#define STRING_TASK_DEPEND_ITSELF    "Una tarea no puede depender de sí misma."
#define STRING_TASK_DEPEND_MISS_CREA "No se pudo crear una dependencia de la tarea {1} - no encontrada."
#define STRING_TASK_DEPEND_MISS_DEL  "No se pudo eliminar una dependencia de la tarea {1} - no encontrada."
#define STRING_TASK_DEPEND_DUP       "La tarea {1} ya depende de la tarea {2}."
#define STRING_TASK_DEPEND_CIRCULAR  "Dependencia circular detectada y anulada."
#define STRING_TASK_VALID_BLANK      "No se puede añadir una tarea que está en blanco."
#define STRING_TASK_VALID_BEFORE     "Advertencia: ha especificado que la fecha '{1}' es después de la fecha '{2}'."
#define STRING_TASK_VALID_REC_DUE    "Una tarea recurrente debe tener también una fecha de vencimiento."
#define STRING_TASK_SAFETY_VALVE     "This command has no filter, and will modify all (including completed and deleted) tasks.  Are you sure?"
#define STRING_TASK_SAFETY_FAIL      "Se impidió la ejecución del comando."
#define STRING_TASK_SAFETY_ALLOW     "No especificó un filtro, y con el valor de 'allow.empty.filter', no se toma ninguna acción."
#define STRING_TASK_INVALID_COL_TYPE "Tipo de columna no reconocido '{1}' para la columna '{2}'"

#endif
