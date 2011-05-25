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
// Strings that should be localized:
//   - text output that the user sees or types
//
// Strings that should NOT be localized:
//   - ./taskrc configuration variable names
//   - command names
//   - extension function names
//   - certain literals associated with parsing
//   - debug strings
//   - attribute names
//   - modifier names
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_EN_US
#define INCLUDED_EN_US

// To localize, clone and rename this file, then change all the defines below.

// Errors
#define STRING_UNKNOWN_ERROR   "Unknown error."
#define STRING_NO_HOME         "Could not read home directory from the passwd file."
#define STRING_TAGS_NO_COMMAS  "Tags are not permitted to contain commas."
#define STRING_TRIVIAL_INPUT   "You must specify a command, or a task ID to modify."
#define STRING_ASSUME_INFO     "No command - assuming 'info'"

// 'show' command
#define STRING_CMD_SHOW        "Shows the entire task configuration variables or the ones containing substring."
#define STRING_CMD_SHOW_ARGS   "You can only specify 'all' or a search string."
#define STRING_CMD_SHOW_NONE   "No matching configuration variables."
#define STRING_CMD_SHOW_UNREC  "Your .taskrc file contains these unrecognized variables:"
#define STRING_CMD_SHOW_DIFFER "Some of your .taskrc variables differ from the default values."
#define STRING_CMD_SHOW_HOOKS  "Your .taskrc file contains these missing or unreadable hook scripts:"
#define STRING_CMD_SHOW_EMPTY  "Configuration error: .taskrc contains no entries."

// DOM
#define STRING_DOM_UNKNOWN     "<unknown>"
#define STRING_DOM_UNREC       "DOM: Cannot get unrecognized name '{1}'."
#define STRING_DOM_CANNOT_SET  "DOM: Cannot set '{1}'."

#endif

