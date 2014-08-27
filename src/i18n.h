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
#if PACKAGE_LANGUAGE == LANGUAGE_ENG_USA
#include <eng-USA.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_ESP_ESP
#include <esp-ESP.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_FRA_FRA
#include <fra-FRA.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_DEU_DEU
#include <deu-DEU.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_ITA_ITA
#include <ita-ITA.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_POR_PRT
#include <por-PRT.h>
#elif PACKAGE_LANGUAGE == LANGUAGE_EPO_RUS
#include <epo-RUS.h>
#endif

#endif

