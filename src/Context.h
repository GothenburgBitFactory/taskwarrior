////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_CONTEXT
#define INCLUDED_CONTEXT

#include <Command.h>
#include <Column.h>
#include <Config.h>
#include <Task.h>
#include <TDB2.h>
#include <Hooks.h>
#include <FS.h>
#include <CLI2.h>
#include <Timer.h>
#include <set>

class Context
{
public:
  Context ();                          // Default constructor
  ~Context ();                         // Destructor

  Context (const Context&);
  Context& operator= (const Context&);

  int initialize (int, const char**);  // all startup
  int run ();
  int dispatch (std::string&);         // command handler dispatch

  int getWidth ();                     // determine terminal width
  int getHeight ();                    // determine terminal height

  const std::vector <std::string> getColumns () const;
  void getLimits (int&, int&);

  bool color ();                       // TTY or <other>?
  bool verbose (const std::string&);   // Verbosity control

  void header (const std::string&);    // Header message sink
  void footnote (const std::string&);  // Footnote message sink
  void debug (const std::string&);     // Debug message sink
  void error (const std::string&);     // Error message sink - non-maskable

  void decomposeSortField (const std::string&, std::string&, bool&, bool&);

private:
  void staticInitialization ();
  void createDefaultConfig ();
  void updateXtermTitle ();
  void updateVerbosity ();
  void loadAliases ();
  void propagateDebug ();

public:
  CLI2                                cli2;
  std::string                         home_dir;
  File                                rc_file;
  Path                                data_dir;
  Config                              config;

  TDB2                                tdb2;
  Hooks                               hooks;

  bool                                determine_color_use;
  bool                                use_color;

  bool                                run_gc;

  bool                                verbosity_legacy;
  std::set <std::string>              verbosity;
  std::vector <std::string>           headers;
  std::vector <std::string>           footnotes;
  std::vector <std::string>           errors;
  std::vector <std::string>           debugMessages;

  std::map <std::string, Command*>    commands;
  std::map <std::string, Column*>     columns;

  int                                 terminal_width;
  int                                 terminal_height;

  Timer                               timer_total;
  Timer                               timer_init;
  Timer                               timer_load;
  Timer                               timer_gc;
  Timer                               timer_filter;
  Timer                               timer_commit;
  Timer                               timer_sort;
  Timer                               timer_render;
  Timer                               timer_hooks;
};

#endif
////////////////////////////////////////////////////////////////////////////////
