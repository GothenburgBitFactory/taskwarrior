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
#define STRING_COLUMN_LABEL_DEP      "Depends"
#define STRING_COLUMN_LABEL_DEP_S    "Dep"
#define STRING_COLUMN_LABEL_DESC     "Description"
#define STRING_COLUMN_LABEL_DUE      "Due"
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
#define STRING_COLUMN_LABEL_RECUR    "Recur"
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
#define STRING_CMD_URGENCY_NO_TASKS  "No tasks specified."
#define STRING_CMD_URGENCY_RESULT    "task {1} urgency {2}"
#define STRING_CMD_ADD_USAGE         "Adds a new task."
#define STRING_CMD_ADD_FEEDBACK      "Created task {1}."
#define STRING_CMD_MOD_UNEXPECTED    "Unexpected argument '{1}' found while modifying a task."
#define STRING_CMD_LOG_USAGE         "Adds a new task that is already completed."

// Config
#define STRING_CONFIG_OVERNEST       "Configuration file nested to more than 10 levels deep - this has to be a mistake."

// Context
#define STRING_CONTEXT_CREATE_RC     "A configuration file could not be found in {1}\n\nWould you like a sample {2} created, so taskwarrior can proceed?"
#define STRING_CONTEXT_NEED_RC       "Cannot proceed without rc file."

// DOM
#define STRING_DOM_UNKNOWN           "<unknown>"
#define STRING_DOM_UNREC             "DOM: Cannot get unrecognized name '{1}'."
#define STRING_DOM_CANNOT_SET        "DOM: Cannot set '{1}'."

// Errors
// TODO Move each of these to appropriate section.
#define STRING_UNKNOWN_ERROR         "Unknown error."
#define STRING_NO_HOME               "Could not read home directory from the passwd file."
#define STRING_TAGS_NO_COMMAS        "Tags are not permitted to contain commas."
#define STRING_TRIVIAL_INPUT         "You must specify a command, or a task ID to modify."
#define STRING_ASSUME_INFO           "No command - assuming 'info'"

// File
#define STRING_FILE_PERMS            "Task does not have the correct permissions for '{1}'."

// interactive
#define STRING_INTERACTIVE_WIDTH     "Context::getWidth: determined width of {1} characters"
#define STRING_INTERACTIVE_HEIGHT    "Context::getHeight: determined height of {1} characters"

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

// Taskmod
#define STRING_TASKMOD_BAD_INIT      "Taskmod::getUuid(): Task object not initialized."
#define STRING_TASKMOD_TIME          "time "
#define STRING_TASKMOD_OLD           "old "
#define STRING_TASKMOD_NEW           "new "

// text
                                     // A comma-separated list of commands is appended.
#define STRING_TEXT_AMBIGUOUS        "Ambiguous {1} '{2}' - could be either of "

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

#endif

