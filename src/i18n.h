////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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

#include <cmake.h>

// Translators:
//   Add more, as appropriate.
#if PACKAGE_LANGUAGE == LANGUAGE_EN_US
#include <en-US.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_ES_ES
#include <es-ES.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_FR_FR
#include <fr-FR.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_DE_DE
#include <de-DE.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_IT_IT
#include <it-IT.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_POR_PRT
#include <por-PRT.h>
#endif

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

