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
//   - All text output that the user sees or types
//
// Strings that should NOT be localized:
//   - ./taskrc configuration variable names
//   - certain literals associated with parsing
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_I18N
#define INCLUDED_I18N
#define L10N                                           // Localization complete.

#include "../cmake.h"

#if PACKAGE_LANGUAGE == LANGUAGE_EN_US
#include <en-US.h>
#endif

// Other languages here.

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

#endif

