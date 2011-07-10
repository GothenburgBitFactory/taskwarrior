////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
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
//   - ./taskrc configuration variable names
//   - command names
//   - extension function names
//   - certain literals associated with parsing
//   - debug strings
//   - attribute names
//   - modifier names
//   - logical operators (and, or, xor)
//
// Rules:
//   - Localized strings should contain leading or trailing white space,
//     including \n, thus allowing the code to compose strings.
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
//   3. Add your new translation to the task.git/src/i18n.h file by changing:
//
//        #if PACKAGE_LANGUAGE == LANGUAGE_EN_US
//        #include <en-US.h>
//        #endif
//
//      to:
//
//        #if PACKAGE_LANGUAGE == LANGUAGE_EN_US
//        #include <en-US.h>
//        #elif PACKAGE_LANGUAGE == LANGUAGE_DE_DE
//        #include <de-DE.h>
//        #endif
//
//   4. Build your localized Taskwarrior with these commands:
//
//      cd task.git
//      cmake -D PACKAGE_LANGUAGE=LANGUAGE_DE_DE .
//      make
//
//   5. Submit your translation to support@taskwarrior.org, where it will be
//      shared with others.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_STRINGS
#define INCLUDED_STRINGS
#define L10N                                           // Localization complete.

// API
#define STRING_API_EXITING           "Exiting."
#define STRING_API_NOFUNC            "The Lua function '{1}' was not found."
#define STRING_API_ERROR_CALLING     "Error calling '{1}' - {2}."
#define STRING_API_ERROR_FAIL        "Error: '{1}' did not return a success indicator."
#define STRING_API_ERROR_NORET       "Error: '{1}' did not return a message or nil."
#define STRING_API_WARNING           "Warning: {1}"
#define STRING_API_ERROR             "Error: {1}"

// Color
#define STRING_COLOR_UNRECOGNIZED    "The color '{1}' is not recognized."

// columns/Col*
#define STRING_COLUMN_BAD_NAME       "Unrecognized column name '{1}'"
#define STRING_COLUMN_BAD_FORMAT     "Unrecognized column format '{1}.{2}'"
#define STRING_COLUMN_LABEL_TASKS    "Tasks"
#define STRING_COLUMN_LABEL_DEP      "Depends"
#define STRING_COLUMN_LABEL_DEP_S    "Dep"
#define STRING_COLUMN_LABEL_DESC     "Description"
#define STRING_COLUMN_LABEL_DUE      "Due"
#define STRING_COLUMN_LABEL_END      "End"
#define STRING_COLUMN_LABEL_ENTERED  "Entered"
#define STRING_COLUMN_LABEL_COUNT    "Count"
#define STRING_COLUMN_LABEL_COMPLETE "Completed"
#define STRING_COLUMN_LABEL_ADDED    "Added"
#define STRING_COLUMN_LABEL_AGE      "Age"
#define STRING_COLUMN_LABEL_ID       "ID"
#define STRING_COLUMN_LABEL_PRI      "Pri"
#define STRING_COLUMN_LABEL_PRIORITY "Priority"
#define STRING_COLUMN_LABEL_PROJECT  "Project"
#define STRING_COLUMN_LABEL_UNTIL    "Until"
#define STRING_COLUMN_LABEL_WAIT     "Wait"
#define STRING_COLUMN_LABEL_WAITING  "Waiting until"
#define STRING_COLUMN_LABEL_RECUR    "Recur"
#define STRING_COLUMN_LABEL_RECUR_L  "Recurrence"
#define STRING_COLUMN_LABEL_START    "Start"
#define STRING_COLUMN_LABEL_STARTED  "Started"
#define STRING_COLUMN_LABEL_ACTIVE   "A"
#define STRING_COLUMN_LABEL_STATUS   "Status"
#define STRING_COLUMN_LABEL_STAT     "St"
#define STRING_COLUMN_LABEL_STAT_PE  "Pending"
#define STRING_COLUMN_LABEL_STAT_CO  "Completed"
#define STRING_COLUMN_LABEL_STAT_DE  "Deleted"
#define STRING_COLUMN_LABEL_STAT_WA  "Waiting"
#define STRING_COLUMN_LABEL_STAT_RE  "Recurring"
#define STRING_COLUMN_LABEL_STAT_P   "P"
#define STRING_COLUMN_LABEL_STAT_C   "C"
#define STRING_COLUMN_LABEL_STAT_D   "D"
#define STRING_COLUMN_LABEL_STAT_W   "W"
#define STRING_COLUMN_LABEL_STAT_R   "R"
#define STRING_COLUMN_LABEL_TAGS     "Tags"
#define STRING_COLUMN_LABEL_TAG      "Tag"
#define STRING_COLUMN_LABEL_UUID     "UUID"
#define STRING_COLUMN_LABEL_URGENCY  "Urgency"
#define STRING_COLUMN_LABEL_NAME     "Name"
#define STRING_COLUMN_LABEL_VALUE    "Value"
#define STRING_COLUMN_LABEL_MASK     "Mask"
#define STRING_COLUMN_LABEL_MASK_IDX "Mask Index"
#define STRING_COLUMN_LABEL_PARENT   "Parent task"
#define STRING_COLUMN_LABEL_FG       "Foreground color"
#define STRING_COLUMN_LABEL_BG       "Background color"
#define STRING_COLUMN_LABEL_DATE     "Date"

// commands/Cmd*
#define STRING_CMD_CONFLICT          "Custom report '{1}' conflicts with built-in task command."
#define STRING_CMD_VERSION_USAGE     "Shows the taskwarrior version number."
#define STRING_CMD_VERSION_USAGE2    "Shows only the taskwarrior version number."
#define STRING_CMD_VERSION_GPL       "Taskwarrior may be copied only under the terms of the GNU General Public License, which may be found in the taskwarrior source kit."
#define STRING_CMD_VERSION_DOCS      "Documentation for taskwarrior can be found using 'man task', 'man taskrc', 'man task-tutorial', 'man task-color', 'man task-sync', 'man task-faq' or at http://taskwarrior.org"
#define STRING_CMD_VERSION_BUILT     "{1} {2} built for "
#define STRING_CMD_VERSION_UNKNOWN   "unknown"
#define STRING_CMD_VERSION_COPY      "Copyright (C) 2006 - 2011 P. Beckingham, F. Hernandez."
#define STRING_CMD_VERSION_COPY2     "Portions of this software Copyright (C) 1994 – 2008 Lua.org, PUC-Rio."
#define STRING_CMD_LOGO_USAGE        "Displays the Taskwarrior logo"
#define STRING_CMD_LOGO_COLOR_REQ    "The logo command requires that color support is enabled."
#define STRING_CMD_EXEC_USAGE        "Executes external commands and scripts"
#define STRING_CMD_URGENCY_USAGE     "Displays the urgency measure of a task."
#define STRING_CMD_URGENCY_RESULT    "task {1} urgency {2}"
#define STRING_CMD_ADD_USAGE         "Adds a new task."
#define STRING_CMD_ADD_FEEDBACK      "Created task {1}."
#define STRING_CMD_ADD_BAD_ATTRIBUTE "Unrecognized attribute '{1}'."
#define STRING_CMD_MOD_UNEXPECTED    "Unexpected argument '{1}' found while modifying a task."
#define STRING_CMD_LOG_USAGE         "Adds a new task that is already completed."
#define STRING_CMD_LOG_NO_RECUR      "You cannot log recurring tasks."
#define STRING_CMD_LOG_NO_WAITING    "You cannot log waiting tasks."
#define STRING_CMD_LOG_LOGGED        "Logged task."
#define STRING_CMD_IDS_USAGE_RANGE   "Shows only the IDs of matching tasks, in the form of a range."
#define STRING_CMD_IDS_USAGE_LIST    "Shows only the IDs of matching tasks, in the form of a list."
#define STRING_CMD_IDS_USAGE_ZSH     "Shows the IDs and descriptions of matching tasks."
#define STRING_CMD_QUERY_USAGE       "Executes external commands and scripts"
#define STRING_CMD_INFO_USAGE        "Shows all data and metadata for specified tasks."
#define STRING_CMD_INFO_BLOCKED      "This task blocked by"
#define STRING_CMD_INFO_BLOCKING     "This task is blocking"
#define STRING_CMD_INFO_RECUR_UNTIL  "Recur until"
#define STRING_CMD_INFO_MODIFICATION "Modification"
#define STRING_CMD_INFO_TOTAL_ACTIVE "Total active time"
#define STRING_CMD_UNDO_USAGE        "Reverts the most recent change to a task."
#define STRING_CMD_STATS_USAGE       "Shows task database statistics."
#define STRING_CMD_STATS_CATEGORY    "Category"
#define STRING_CMD_STATS_DATA        "Data"
#define STRING_CMD_STATS_TOTAL       "Total"
#define STRING_CMD_STATS_ANNOTATIONS "Annotations"
#define STRING_CMD_STATS_UNIQUE_TAGS "Unique tags"
#define STRING_CMD_STATS_PROJECTS    "Projects"
#define STRING_CMD_STATS_DATA_SIZE   "Data size"
#define STRING_CMD_STATS_UNDO_TXNS   "Undo transactions"
#define STRING_CMD_STATS_TAGGED      "Tasks tagged"
#define STRING_CMD_STATS_OLDEST      "Oldest task"
#define STRING_CMD_STATS_NEWEST      "Newest task"
#define STRING_CMD_STATS_USED_FOR    "Task used for"
#define STRING_CMD_STATS_ADD_EVERY   "Task added every"
#define STRING_CMD_STATS_COMP_EVERY  "Task completed every"
#define STRING_CMD_STATS_DEL_EVERY   "Task deleted every"
#define STRING_CMD_STATS_AVG_PEND    "Average time pending"
#define STRING_CMD_STATS_DESC_LEN    "Average desc length"
#define STRING_CMD_STATS_CHARS       "{1} characters"
#define STRING_CMD_REPORTS_USAGE     "Lists all supported reports."
#define STRING_CMD_REPORTS_REPORT    "Report"
#define STRING_CMD_REPORTS_DESC      "Description"
#define STRING_CMD_REPORTS_SUMMARY   "{1} reports"
#define STRING_CMD_TAGS_USAGE        "Shows a list of all tags used."
#define STRING_CMD_COMTAGS_USAGE     "Shows only a list of all tags used, for autocompletion purposes."
#define STRING_CMD_TAGS_SINGLE       "1 tag"
#define STRING_CMD_TAGS_PLURAL       "{1} tags"
#define STRING_CMD_TAGS_NO_TAGS      "No tags."
#define STRING_CMD_HISTORY_USAGE_M   "Shows a report of task history, by month."
#define STRING_CMD_HISTORY_YEAR      "Year"
#define STRING_CMD_HISTORY_MONTH     "Month"
#define STRING_CMD_HISTORY_ADDED     "Added"
#define STRING_CMD_HISTORY_COMP      "Completed"
#define STRING_CMD_HISTORY_DEL       "Deleted"
#define STRING_CMD_HISTORY_NET       "Net"
#define STRING_CMD_HISTORY_USAGE_A   "Shows a report of task history, by year."
#define STRING_CMD_HISTORY_AVERAGE   "Average"
#define STRING_CMD_HISTORY_LEGEND    "Legend: {1}, {2}, {3}"
#define STRING_CMD_HISTORY_LEGEND_A  "Legend: + added, X completed, - deleted"
#define STRING_CMD_GHISTORY_USAGE_M  "Shows a graphical report of task history, by month."
#define STRING_CMD_GHISTORY_USAGE_A  "Shows a graphical report of task history, by year."
#define STRING_CMD_GHISTORY_YEAR     "Year"
#define STRING_CMD_GHISTORY_MONTH    "Month"
#define STRING_CMD_GHISTORY_NUMBER   "Number Added/Completed/Deleted"
#define STRING_CMD_DONE_USAGE        "Marks the specified task as completed."
#define STRING_CMD_DONE_PROCEED      "Proceed with change?"
#define STRING_CMD_DONE_COMPLETED    "Completed {1} '{2}'."
#define STRING_CMD_DONE_NOT_PENDING  "Task {1} '{2}' is neither pending nor waiting."
#define STRING_CMD_DONE_MARKED       "Marked {1} task as done"
#define STRING_CMD_DONE_MARKED_N     "Marked {1} tasks as done"
#define STRING_CMD_PROJECTS_USAGE    "Shows a list of all project names used, and how many tasks are in each"
#define STRING_CMD_PROJECTS_USAGE_2  "Shows only a list of all project names used"
#define STRING_CMD_PROJECTS_NO       "No projects."
#define STRING_CMD_PROJECTS_PRI_N    "Pri:None"
#define STRING_CMD_PROJECTS_PRI_H    "Pri:H"
#define STRING_CMD_PROJECTS_PRI_M    "Pri:M"
#define STRING_CMD_PROJECTS_PRI_L    "Pri:L"
#define STRING_CMD_PROJECTS_NONE     "(none)"
#define STRING_CMD_PROJECTS_SUMMARY  "{1} project"
#define STRING_CMD_PROJECTS_SUMMARY2 "{1} projects"
#define STRING_CMD_PROJECTS_TASK     "({1} task)"
#define STRING_CMD_PROJECTS_TASKS    "({1} tasks)"
#define STRING_CMD_SUMMARY_USAGE     "Shows a report of task status by project."
#define STRING_CMD_SUMMARY_PROJECT   "Project"
#define STRING_CMD_SUMMARY_REMAINING "Remaining"
#define STRING_CMD_SUMMARY_AVG_AGE   "Avg age"
#define STRING_CMD_SUMMARY_COMPLETE  "Complete"
#define STRING_CMD_COUNT_USAGE       "Shows only the number of matching tasks."
#define STRING_CMD_DELETE_USAGE      "Deletes the specified task."
#define STRING_CMD_DELETE_QUESTION   "Permanently delete task {1} '{2}'?"
#define STRING_CMD_DELETE_RECURRING  "Deleting recurring task {1} '{2}'."
#define STRING_CMD_DELETE_DELETING   "Deleting task {1} '{2}'."
#define STRING_CMD_DELETE_CONF_RECUR "This is a recurring task.  Do you want to delete all pending recurrences of this same task?"
#define STRING_CMD_DELETE_NOT        "Task not deleted."
#define STRING_CMD_DELETE_NOTPEND    "Task {1} '{2}' is neither pending nor waiting."
#define STRING_CMD_DUPLICATE_USAGE   "Duplicates the specified tasks, and allows modifications."
#define STRING_CMD_DUPLICATE_NON_REC "Note: task {1} was a recurring task.  The duplicate task is not."
#define STRING_CMD_DUPLICATE_DONE    "Duplicated {1} '{2}'."
#define STRING_CMD_START_USAGE       "Marks specified task as started."
#define STRING_CMD_START_DONE        "Started {1} '{2}'."
#define STRING_CMD_START_ALREADY     "Task {1} '{2}' already started."
#define STRING_CMD_STOP_USAGE        "Removes the 'start' time from a task."
#define STRING_CMD_STOP_NOT          "Task {1} '{2}' not started."
#define STRING_CMD_STOP_DONE         "Stopped {1} '{2}'."
#define STRING_CMD_APPEND_USAGE      "Appends text to an existing task description."
#define STRING_CMD_APPEND_DONE       "Appended to task {1}."
#define STRING_CMD_APPEND_SUMMARY    "Appended {1} task."
#define STRING_CMD_APPEND_SUMMARY_N  "Appended {1} tasks."
#define STRING_CMD_PREPEND_USAGE     "Prepends text to an existing task description."
#define STRING_CMD_PREPEND_DONE      "Prepended to task {1}."
#define STRING_CMD_PREPEND_SUMMARY   "Prepended {1} task."
#define STRING_CMD_PREPEND_SUMMARY_N "Prepended {1} tasks."
#define STRING_CMD_XPEND_NEED_TEXT   "Additional text must be provided."

// Config
#define STRING_CONFIG_OVERNEST       "Configuration file nested to more than 10 levels deep - this has to be a mistake."
#define STRING_CONFIG_READ_INCLUDE   "Could not read include file '{1}'."
#define STRING_CONFIG_INCLUDE_PATH   "Can only include files with absolute paths, not '{1}'"
#define STRING_CONFIG_BAD_ENTRY      "Malformed entry '{1}'."
#define STRING_CONFIG_BAD_WRITE      "Could not write to '{1}'."
#define STRING_CONFIG_DEPRECATED_US  "Your .taskrc file contains color settings that use deprecated underscores.  Please check:"
#define STRING_CONFIG_DEPRECATED_COL "Your .taskrc file contains reports with deprecated columns.  Please check for entry_time, start_time or end_time in:"

// Context
#define STRING_CONTEXT_CREATE_RC     "A configuration file could not be found in {1}\n\nWould you like a sample {2} created, so taskwarrior can proceed?"
#define STRING_CONTEXT_NEED_RC       "Cannot proceed without rc file."

// Date
#define STRING_DATE_INVALID_FORMAT   "'{1}' is not a valid date in the '{2}' format."
#define STRING_DATE_BAD_WEEKSTART    "The 'weekstart' configuration variable may only contain 'Sunday' or 'Monday'."

// dependency
#define STRING_DEPEND_BLOCKED        "Task {1} is blocked by:"
#define STRING_DEPEND_BLOCKING       "and is blocking:"
#define STRING_DEPEND_FIX_CHAIN      "Would you like the dependency chain fixed?"

// DOM
#define STRING_DOM_UNKNOWN           "<unknown>"
#define STRING_DOM_UNREC             "DOM: Cannot get unrecognized name '{1}'."
#define STRING_DOM_CANNOT_SET        "DOM: Cannot set '{1}'."

// Duration
#define STRING_DURATION_UNRECOGNIZED "The duration '{1}' was not recognized."

// Errors
// TODO Move each of these to appropriate section.
#define STRING_UNKNOWN_ERROR         "Unknown error."
#define STRING_NO_HOME               "Could not read home directory from the passwd file."
#define STRING_TAGS_NO_COMMAS        "Tags are not permitted to contain commas."
#define STRING_TRIVIAL_INPUT         "You must specify a command, or a task ID to modify."
#define STRING_ASSUME_INFO           "No command specified - assuming 'information'"
#define STRING_INFINITE_LOOP         "Terminated substitution because more than {1} changes were made - infinite loop protection."

// Feedback
#define STRING_FEEDBACK_NO_TASKS     "No tasks."
#define STRING_FEEDBACK_NO_TASKS_SP  "No tasks specified."
#define STRING_FEEDBACK_NO_MATCH     "No matches."
#define STRING_FEEDBACK_TASKS_SINGLE "(1 task)"
#define STRING_FEEDBACK_TASKS_PLURAL "({1} tasks)"

// File
#define STRING_FILE_PERMS            "Task does not have the correct permissions for '{1}'."

// interactive
#define STRING_INTERACTIVE_WIDTH     "Context::getWidth: determined width of {1} characters"
#define STRING_INTERACTIVE_HEIGHT    "Context::getHeight: determined height of {1} characters"

// JSON
#define STRING_JSON_MISSING_VALUE    "Error: missing value after ',' at position {1}"
#define STRING_JSON_MISSING_VALUE2   "Error: missing value at position {1}"
#define STRING_JSON_MISSING_BRACKET  "Error: missing ']' at position {1}"
#define STRING_JSON_MISSING_BRACE    "Error: missing '}' at position {1}"
#define STRING_JSON_MISSING_COLON    "Error: missing ':' at position {1}"
#define STRING_JSON_MISSING_OPEN     "Error: expected '{' or '[' at position {1}"
#define STRING_JSON_EXTRA_CHARACTERS "Error: extra characters found at position {1}"

// Lua
#define STRING_LUA_BAD_HOOK_DEF      "Malformed hook definition '{1}'."
#define STRING_LUA_BAD_EVENT         "Unrecognized hook event '{1}'."

// Permission
#define STRING_PERM_TASK_LINE        "task {1} \"{2}\""
#define STRING_PERM_RECURRING        "(Recurring)"

// Record
#define STRING_RECORD_EMPTY          "Empty record in input."
#define STRING_RECORD_JUNK_AT_EOL    "Unrecognized characters at end of line."
#define STRING_RECORD_NOT_FF4        "Record not recognized as format 4."

// recur
#define STRING_RECUR_PAST_UNTIL      "Task ({1}) has past its 'until' date, and has been deleted."

// 'show' command
#define STRING_CMD_SHOW              "Shows the entire task configuration variables or the ones containing substring."
#define STRING_CMD_SHOW_ARGS         "You can only specify 'all' or a search string."
#define STRING_CMD_SHOW_NONE         "No matching configuration variables."
#define STRING_CMD_SHOW_UNREC        "Your .taskrc file contains these unrecognized variables:"
#define STRING_CMD_SHOW_DIFFER       "Some of your .taskrc variables differ from the default values."
#define STRING_CMD_SHOW_HOOKS        "Your .taskrc file contains these missing or unreadable hook scripts:"
#define STRING_CMD_SHOW_EMPTY        "Configuration error: .taskrc contains no entries."
#define STRING_CMD_SHOW_DIFFER_COLOR "These are highlighted in {1} above."
#define STRING_CMD_SHOW_CONFIG_ERROR "Configuration error: {1} contains an unrecognized value '{2}'."
#define STRING_CMD_SHOW_NO_LOCATION  "Configuration error: data.location not specified in .taskrc file."
#define STRING_CMD_SHOW_LOC_EXIST    "Configuration error: data.location contains a directory name that doesn't exist, or is unreadable."
#define STRING_CMD_SHOW_CONF_VAR     "Config Variable"
#define STRING_CMD_SHOW_CONF_VALUE   "Value"

// Task
#define STRING_TASK_NO_FF1           "Taskwarrior no longer supports file format 1, originally used between 27 November 2006 and 31 December 2007."
#define STRING_TASK_PARSE_ANNO_BRACK "Missing annotation brackets."
#define STRING_TASK_PARSE_ATT_BRACK  "Missing attribute brackets."
#define STRING_TASK_PARSE_TAG_BRACK  "Missing tag brackets."
#define STRING_TASK_PARSE_TOO_SHORT  "Line too short."
#define STRING_TASK_PARSE_UNREC_FF   "Unrecognized taskwarrior file format."
#define STRING_TASK_DEPEND_ITSELF    "A task cannot be dependent on itself."
#define STRING_TASK_DEPEND_MISSING   "Could not create a dependency on task {1} - not found."
#define STRING_TASK_DEPEND_DUP       "Task {1} already depends on task {2}."
#define STRING_TASK_DEPEND_CIRCULAR  "Circular dependency detected and disallowed."
#define STRING_TASK_DEPEND_NO_UUID   "Could not find a UUID for id {1}."
#define STRING_TASK_VALID_UUID       "A task must have a UUID."
#define STRING_TASK_VALID_ENTRY      "A task must have an entry timestamp."
#define STRING_TASK_VALID_DESC       "A task must have a description."
#define STRING_TASK_VALID_BLANK      "Cannot add a task that is blank."
#define STRING_TASK_VALID_WAIT       "A 'wait' date must be before a 'due' date."
#define STRING_TASK_VALID_START      "A 'start' date must be after an 'entry' date."
#define STRING_TASK_VALID_END        "An 'end' date must be after an 'entry' date."
#define STRING_TASK_VALID_REC_DUE    "You cannot specify a recurring task without a due date."
#define STRING_TASK_VALID_UNTIL      "You cannot specify an until date for a non-recurring task."
#define STRING_TASK_VALID_RECUR      "A recurrence value must be valid."
#define STRING_TASK_VALID_WAIT_RECUR "You cannot create a task that is both waiting and recurring."


// Taskmod
#define STRING_TASKMOD_BAD_INIT      "Taskmod::getUuid(): Task object not initialized."
#define STRING_TASKMOD_TIME          "time "
#define STRING_TASKMOD_OLD           "old "
#define STRING_TASKMOD_NEW           "new "

// text
                                     // A comma-separated list of commands is appended.
#define STRING_TEXT_AMBIGUOUS        "Ambiguous {1} '{2}' - could be either of "

// Transport
#define STRING_TRANSPORT_URI_NODIR   "The uri '{1}' does not appear to be a directory."
#define STRING_TRANSPORT_CURL_URI    "When using the 'curl' protocol, the uri must contain a hostname."
#define STRING_TRANSPORT_CURL_WILDCD "When using the 'curl' protocol, wildcards are not supported."
#define STRING_TRANSPORT_CURL_NORUN  "Could not run curl.  Is it installed, and available in $PATH?"
#define STRING_TRANSPORT_CURL_FAIL   "Curl failed, see output above."
#define STRING_TRANSPORT_RSYNC_URI   "When using the 'rsync' protocol, the uri must contain a hostname."
#define STRING_TRANSPORT_RSYNC_NORUN "Could not run rsync.  Is it installed, and available in $PATH?"
#define STRING_TRANSPORT_SSH_URI     "When using the 'ssh' protocol, the uri must contain a hostname."
#define STRING_TRANSPORT_SSH_NORUN   "Could not run ssh.  Is it installed, and available in $PATH?"

// Uri
#define STRING_URI_QUOTES            "Could not parse uri '{1}', wrong usage of single quotes."
#define STRING_URI_BAD_FORMAT        "The uri '{1}' is not in the expected format."

// utf8
#define STRING_UTF8_INVALID_CP_REP   "Invalid codepoint representation."
#define STRING_UTF8_INVALID_CP       "Invalid Unicode codepoint."

// util
#define STRING_UTIL_CONFIRM_YN       " (y/n) "
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

// Variant
#define STRING_VARIANT_REL_BOOL      "Cannot perform relational comparison on Boolean types."
#define STRING_VARIANT_REL_UNKNOWN   "Cannot perform relational comparison on unknown types."

#endif

