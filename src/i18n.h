////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#define CMD_EXPORT              211
#define CMD_HELP                212
#define CMD_HISTORY             213
#define CMD_GHISTORY            214
#define CMD_IMPORT              215
#define CMD_INFO                216
#define CMD_NEXT                217
#define CMD_OVERDUE             218
#define CMD_PROJECTS            219
#define CMD_START               220
#define CMD_STATS               221
#define CMD_STOP                222
#define CMD_SUMMARY             223
#define CMD_TAGS                224
#define CMD_TIMESHEET           225
#define CMD_UNDELETE            226
#define CMD_UNDO                227
#define CMD_VERSION             228

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
#define COLOR_BOLD              500
#define COLOR_UL                501
#define COLOR_B_UL              502
#define COLOR_BLACK             503
#define COLOR_RED               504
#define COLOR_GREEN             505
#define COLOR_YELLOW            506
#define COLOR_BLUE              507
#define COLOR_MAGENTA           508
#define COLOR_CYAN              509
#define COLOR_WHITE             510
#define COLOR_B_BLACK           511
#define COLOR_B_RED             512
#define COLOR_B_GREEN           513
#define COLOR_B_YELLOW          514
#define COLOR_B_BLUE            515
#define COLOR_B_MAGENTA         516
#define COLOR_B_CYAN            517
#define COLOR_B_WHITE           518
#define COLOR_UL_BLACK          519
#define COLOR_UL_RED            520
#define COLOR_UL_GREEN          521
#define COLOR_UL_YELLOW         522
#define COLOR_UL_BLUE           523
#define COLOR_UL_MAGENTA        524
#define COLOR_UL_CYAN           525
#define COLOR_UL_WHITE          526
#define COLOR_B_UL_BLACK        527
#define COLOR_B_UL_RED          528
#define COLOR_B_UL_GREEN        529
#define COLOR_B_UL_YELLOW       530
#define COLOR_B_UL_BLUE         531
#define COLOR_B_UL_MAGENTA      532
#define COLOR_B_UL_CYAN         533
#define COLOR_B_UL_WHITE        534
#define COLOR_ON_BLACK          535
#define COLOR_ON_RED            536
#define COLOR_ON_GREEN          537
#define COLOR_ON_YELLOW         538
#define COLOR_ON_BLUE           539
#define COLOR_ON_MAGENTA        540
#define COLOR_ON_CYAN           541
#define COLOR_ON_WHITE          542
#define COLOR_ON_BRIGHT_BLACK   543
#define COLOR_ON_BRIGHT_RED     544
#define COLOR_ON_BRIGHT_GREEN   545
#define COLOR_ON_BRIGHT_YELLOW  546
#define COLOR_ON_BRIGHT_BLUE    547
#define COLOR_ON_BRIGHT_MAGENTA 548
#define COLOR_ON_BRIGHT_CYAN    549
#define COLOR_ON_BRIGHT_WHITE   550

// 6xx Config

// 7xx TDB

// 8xx Reports

#endif

