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

#define STRING_CMD_ALIASES_USAGE     "Gera uma lista com todos os 'alias', para fins de terminação automática"
#define STRING_CMD_CAL_BAD_ARG       "Não foi possível reconhecer o argumento '{1}'."
#define STRING_CMD_CAL_BAD_MONTH     "Argumento '{1}' não é um mês válido."
#define STRING_CMD_CAL_LABEL_DATE    "Data"
#define STRING_CMD_CAL_LABEL_HOL     "Feriado"
#define STRING_CMD_CAL_SUN_MON       "A variável de configuração 'weekstart' apenas pode conter 'domingo' ou 'segunda-feira'."
#define STRING_CMD_CAL_USAGE         "Mostra um calendário, com datas de vencimento assinaladas"
#define STRING_CMD_COMMANDS_USAGE    "Generates a list of all commands, with behavior details"
#define STRING_CMD_COMTAGS_USAGE     "Exibe apenas a lista de todas as marcas em uso, para fins de terminação automática"
#define STRING_CMD_CUSTOM_COUNT      "1 tarefa"
#define STRING_CMD_CUSTOM_COUNTN     "{1} tarefas"
#define STRING_CMD_CUSTOM_MISMATCH   "O número de colunas e de rótulos não é o mesmo no relatório '{1}'."
#define STRING_CMD_CUSTOM_SHOWN      "{1} visiveis"
#define STRING_CMD_CUSTOM_TRUNCATED  "truncado a {1} linhas"
#define STRING_CMD_EXEC_USAGE        "Executa comandos e scripts externos"
#define STRING_CMD_HCOMMANDS_USAGE   "Gera uma lista com todos os comandos, para fins de terminação automática"
#define STRING_CMD_IDS_USAGE_LIST    "Exibe em formato de lista, os IDs das tarefas correspondentes"
#define STRING_CMD_IDS_USAGE_RANGE   "Exibe como intervalo, os IDs das tarefas correspondentes"
#define STRING_CMD_IDS_USAGE_ZSH     "Exibe os IDs e descrições das tarefas correspondentes"
#define STRING_CMD_TAGS_NO_TAGS      "Sem marcas."
#define STRING_CMD_TAGS_PLURAL       "{1} marcas"
#define STRING_CMD_TAGS_SINGLE       "1 marca"
#define STRING_CMD_TAGS_USAGE        "Exibe uma lista de todas as marcas em uso"
#define STRING_CMD_UDAS_COMPL_USAGE  "Exibe os UDAs definidos para fins de terminação automática"
#define STRING_CMD_UDAS_USAGE        "Exibe os detalhes de todas os UDA definidos"
#define STRING_CMD_UNDO_USAGE        "Reverte a mais recente modificação a uma tarefa"
#define STRING_CMD_URGENCY_RESULT    "Tarefa {1} urgência {2}"
#define STRING_CMD_URGENCY_USAGE     "Exibe o valor de urgência de uma tarefa"
#define STRING_CMD_UUIDS_USAGE_LIST  "Exibe como lista, os UUIDs das tarefas correspondentes"
#define STRING_CMD_UUIDS_USAGE_RANGE "Exibe como lista separada por vírgulas, os UUIDs das tarefas correspondentes"
#define STRING_CMD_UUIDS_USAGE_ZSH   "Exibe os UUIDs e descrições das tarefas correspondentes"
#define STRING_CMD_ZSHATTS_USAGE     "Gera uma lista de todos os atributos, para terminação automática em zsh"
#define STRING_CMD_ZSHCOMMANDS_USAGE "Gera uma lista com todos os comandos, para terminação automática em zsh"
#define STRING_COLUMN_EXAMPLES_ANNO1 "Imediatamente antes do almoço"
#define STRING_COLUMN_EXAMPLES_ANNO2 "Se vais jogar no torneio esta tarde"
#define STRING_COLUMN_EXAMPLES_ANNO3 "Antes de escrever para casa"
#define STRING_COLUMN_EXAMPLES_ANNO4 "Se não vais cortar o cabelo"
#define STRING_COLUMN_EXAMPLES_DESC  "Mover as roupas para a prateleira mais baixa"
#define STRING_COLUMN_EXAMPLES_IND   "  casa.jardim"
#define STRING_COLUMN_EXAMPLES_PAR   "casa"
#define STRING_COLUMN_EXAMPLES_PROJ  "casa.jardim"
#define STRING_COLUMN_EXAMPLES_TAGS  "casa @contas próxima"
#define STRING_COLUMN_LABEL_ACTIVE   "A"  // |por-PRT|==|eng-USA|
#define STRING_COLUMN_LABEL_ADDED    "Adicionada"
#define STRING_COLUMN_LABEL_AGE      "Idade"
#define STRING_COLUMN_LABEL_CATEGORY "Category"
#define STRING_COLUMN_LABEL_COLUMN   "Colunas"
#define STRING_COLUMN_LABEL_COMMAND  "Command"
#define STRING_COLUMN_LABEL_COMPLETE "Concluída"
#define STRING_COLUMN_LABEL_CONTEXT  "Context"
#define STRING_COLUMN_LABEL_COUNT    "Contagem"
#define STRING_COLUMN_LABEL_DATE     "Data"
#define STRING_COLUMN_LABEL_DEFAULT  "Por omissão"
#define STRING_COLUMN_LABEL_DESC     "Descrição"
#define STRING_COLUMN_LABEL_DUE      "Vence"
#define STRING_COLUMN_LABEL_END      "Fim"
#define STRING_COLUMN_LABEL_ENTERED  "Criada"
#define STRING_COLUMN_LABEL_EXAMPLES "Exemplo"
#define STRING_COLUMN_LABEL_FILTER   "Filter"
#define STRING_COLUMN_LABEL_GC       "GC"
#define STRING_COLUMN_LABEL_LABEL    "Rótulo"
#define STRING_COLUMN_LABEL_LAST     "Last instance"
#define STRING_COLUMN_LABEL_MISC     "Misc"
#define STRING_COLUMN_LABEL_MOD      "Modificada"
#define STRING_COLUMN_LABEL_MODIFY   "Modifiable"
#define STRING_COLUMN_LABEL_MODS     "Mods"
#define STRING_COLUMN_LABEL_NAME     "Nome"
#define STRING_COLUMN_LABEL_NOMODIFY "Read Only"
#define STRING_COLUMN_LABEL_ORPHAN   "UDA Orfão"
#define STRING_COLUMN_LABEL_PROJECT  "Projeto"
#define STRING_COLUMN_LABEL_RECUR    "Period"
#define STRING_COLUMN_LABEL_RECUR_L  "Periodicidade"
#define STRING_COLUMN_LABEL_RO       "R/W"
#define STRING_COLUMN_LABEL_RTYPE    "Recurrence type"
#define STRING_COLUMN_LABEL_SCHED    "Agendado"
#define STRING_COLUMN_LABEL_SHOWS_ID "ID"
#define STRING_COLUMN_LABEL_START    "Início"
#define STRING_COLUMN_LABEL_STARTED  "Iniciada"
#define STRING_COLUMN_LABEL_STAT     "Es"
#define STRING_COLUMN_LABEL_STATUS   "Estado"
#define STRING_COLUMN_LABEL_STAT_C   "C"  // |por-PRT|==|eng-USA|
#define STRING_COLUMN_LABEL_STAT_CO  "Concluídas"
#define STRING_COLUMN_LABEL_STAT_D   "E"
#define STRING_COLUMN_LABEL_STAT_DE  "Eliminadas"
#define STRING_COLUMN_LABEL_STAT_P   "P"  // |por-PRT|==|eng-USA|
#define STRING_COLUMN_LABEL_STAT_PE  "Pendentes"
#define STRING_COLUMN_LABEL_STAT_R   "R"  // |por-PRT|==|eng-USA|
#define STRING_COLUMN_LABEL_STAT_RE  "Recorrentes"
#define STRING_COLUMN_LABEL_STAT_W   "A"
#define STRING_COLUMN_LABEL_STAT_WA  "Adiadas"
#define STRING_COLUMN_LABEL_STYLES   "Formatos Suportados"
#define STRING_COLUMN_LABEL_TAG      "Marca"
#define STRING_COLUMN_LABEL_TAGS     "Marcas"
#define STRING_COLUMN_LABEL_TYPE     "Tipo"
#define STRING_COLUMN_LABEL_UDA      "Nome"
#define STRING_COLUMN_LABEL_UDACOUNT "Contagem de Uso"
#define STRING_COLUMN_LABEL_UNTIL    "Até"
#define STRING_COLUMN_LABEL_URGENCY  "Urgência"
#define STRING_COLUMN_LABEL_UUID     "UUID"  // |por-PRT|==|eng-USA|
#define STRING_COLUMN_LABEL_VALUE    "Valor"
#define STRING_COLUMN_LABEL_VALUES   "Valores Permitidos"
#define STRING_COLUMN_LABEL_WAIT     "Adiada"
#define STRING_COLUMN_LABEL_WAITING  "Adiada até"
#define STRING_FEEDBACK_ANN_ADD      "Adicionada anotação de '{1}'."
#define STRING_FEEDBACK_ANN_DEL      "Eliminada anotação '{1}'."
#define STRING_FEEDBACK_ANN_WAS_MOD  "Anotação alterada para '{1}'."
#define STRING_FEEDBACK_ATT_DEL      "{1} eliminado."
#define STRING_FEEDBACK_ATT_DEL_DUR  "{1} eliminado (duração: {2})."
#define STRING_FEEDBACK_ATT_MOD      "{1} será alterado de '{2}' para '{3}'."
#define STRING_FEEDBACK_ATT_SET      "{1} será definido como '{2}'."
#define STRING_FEEDBACK_ATT_WAS_MOD  "{1} alterado de '{2}' para '{3}'."
#define STRING_FEEDBACK_ATT_WAS_SET  "{1} definido como '{2}'."
#define STRING_FEEDBACK_BACKLOG      "Há {1} modificações locais. Necessário sincronizar (sync)."
#define STRING_FEEDBACK_BACKLOG_N    "Há {1} modificações locais. Necessário sincronizar (sync)."
#define STRING_FEEDBACK_DELETED      "{1} será eliminada."
#define STRING_FEEDBACK_DEP_DEL      "Eliminadas as dependências '{1}'."
#define STRING_FEEDBACK_DEP_MOD      "As dependências serão alteradas de '{1}' para '{2}'."
#define STRING_FEEDBACK_DEP_SET      "As dependências serão alteradas para '{1}'."
#define STRING_FEEDBACK_DEP_WAS_MOD  "Alteradas as dependências de '{1}' para '{2}'."
#define STRING_FEEDBACK_DEP_WAS_SET  "Alteradas as dependências para '{1}'."
#define STRING_FEEDBACK_EXPIRED      "Tarefa {1} '{2}' expirou e foi eliminada."
#define STRING_FEEDBACK_NOP          "Não serão efetuadas alterações."
#define STRING_FEEDBACK_NO_MATCH     "Nenhuma correspondência."
#define STRING_FEEDBACK_NO_TASKS     "Nenhuma tarefa."
#define STRING_FEEDBACK_TAG_NEXT     "A marca especial 'next' irá aumentar a urgência desta tarefa de modo a que apareça no relatório 'next'."
#define STRING_FEEDBACK_TAG_NOCAL    "A marca especial 'nocal' irá manter esta tarefa ausente do relatório de 'calendário'."
#define STRING_FEEDBACK_TAG_NOCOLOR  "A marca especial 'nocolor' irá desactivar as regras de cor nesta tarefa."
#define STRING_FEEDBACK_TAG_NONAG    "A marca especial 'nonag' irá prevenir avisos quando a tarefa é modificada."
#define STRING_FEEDBACK_TAG_VIRTUAL  "Virtual tags (including '{1}') are reserved and may not be added or removed."
#define STRING_FEEDBACK_TASKS_PLURAL "({1} tarefas)"
#define STRING_FEEDBACK_TASKS_SINGLE "(1 tarefa)"
#define STRING_FEEDBACK_UNBLOCKED    "Desbloqueada {1} '{2}'."
#define STRING_FEEDBACK_WAS_NOP      "Nenhuma alteração efetuada."
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
