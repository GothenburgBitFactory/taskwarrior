////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
//   - All text output that the user sees or types
//
// Strings that should NOT be localized:
//   - ./taskrc configuration variable names
//   - certain literals associated with parsing
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_I18N
#define INCLUDED_I18N

// 1xx task shell
#define SHELL_UNKNOWN_ERROR     100
#define SHELL_READ_PASSWD       101

#define CONFIRM_YES_NO          102

#define SEQUENCE_BAD_SEQ        103
#define SEQUENCE_BAD_RANGE      104
#define SEQUENCE_INVERTED       105
#define SEQUENCE_RANGE_MAX      106
#define SEQUENCE_NOT_A_SEQUENCE 107

#define INTERACTIVE_NO_NCURSES  108

#define RECORD_EMPTY            109
#define RECORD_EXTRA            110
#define RECORD_NOT_FF4          111

#define SUBST_EMPTY             112
#define SUBST_BAD_CHARS         113
#define SUBST_MALFORMED         114

#define TAGS_NO_COMMA           115

#define CMD_MISSING             116

// 2xx Commands
#define CMD_ACTIVE              200
#define CMD_ADD                 201
#define CMD_APPEND              202
#define CMD_ANNOTATE            203
#define CMD_CALENDAR            204
#define CMD_COLORS              205
#define CMD_COMPLETED           206
#define CMD_DELETE              207
#define CMD_DONE                208
#define CMD_DUPLICATE           209
#define CMD_EDIT                210
#define CMD_DENOTATE		211

#define CMD_HELP                212

#define CMD_IMPORT              215
#define CMD_INFO                216
#define CMD_PREPEND             217
#define CMD_OVERDUE             218
#define CMD_PROJECTS            219
#define CMD_START               220
#define CMD_STATS               221
#define CMD_STOP                222
#define CMD_SUMMARY             223
#define CMD_TAGS                224
#define CMD_TIMESHEET           225
#define CMD_LOG                 226
#define CMD_UNDO                227
#define CMD_VERSION             228
#define CMD_SHELL               229
#define CMD_CONFIG              230
#define CMD_SHOW                231
#define CMD_MERGE               232
#define CMD_PUSH                233
#define CMD_PULL                234
#define CMD_DIAGNOSTICS         235

// 3xx Attributes
#define ATT_PROJECT             300
#define ATT_PRIORITY            301
#define ATT_FG                  302
#define ATT_BG                  303
#define ATT_DUE                 304
#define ATT_ENTRY               305
#define ATT_START               306
#define ATT_END                 307
#define ATT_RECUR               308
#define ATT_UNTIL               309
#define ATT_MASK                310
#define ATT_IMASK               311

#define ATT_MOD_BEFORE          350
#define ATT_MOD_AFTER           351
#define ATT_MOD_NOT             352
#define ATT_MOD_NONE            353
#define ATT_MOD_ANY             354
#define ATT_MOD_SYNTH           355
#define ATT_MOD_UNDER           356
#define ATT_MOD_OVER            357
#define ATT_MOD_FIRST           358
#define ATT_MOD_LAST            359
#define ATT_MOD_THIS            360
#define ATT_MOD_NEXT            361
#define ATT_MOD_IS              362
#define ATT_MOD_ISNT            363
#define ATT_MOD_HAS             364
#define ATT_MOD_HASNT           365
#define ATT_MOD_STARTSWITH      366
#define ATT_MOD_ENDSWITH        367

// 4xx Columns

// 5xx Colors
#define CCOLOR_BOLD             500
#define CCOLOR_UNDERLINE        501
#define CCOLOR_ON               502
#define CCOLOR_BRIGHT           503
#define CCOLOR_BLACK            504
#define CCOLOR_RED              505
#define CCOLOR_GREEN            506
#define CCOLOR_YELLOW           507
#define CCOLOR_BLUE             508
#define CCOLOR_MAGENTA          509
#define CCOLOR_CYAN             510
#define CCOLOR_WHITE            511

#define CCOLOR_OFF              520
#define CCOLOR_UNKNOWN          521

// 6xx Config

// 7xx TDB

// 8xx Reports

#endif

