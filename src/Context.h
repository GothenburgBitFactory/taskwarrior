////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_CONTEXT
#define INCLUDED_CONTEXT

#include <Command.h>
#include <Column.h>
#include <Configuration.h>
#include <Task.h>
#include <TDB2.h>
#include <Hooks.h>
#include <FS.h>
#include <CLI2.h>
#include <Timer.h>
#include <set>

class CurrentTask;

class Context
{
public:
  Context () = default;                // Default constructor
  ~Context ();                         // Destructor

  Context (const Context&);
  Context& operator= (const Context&);

  static Context& getContext ();
  static void setContext (Context*);

  int initialize (int, const char**);  // all startup
  int run ();
  int dispatch (std::string&);         // command handler dispatch

  int getWidth ();                     // determine terminal width
  int getHeight ();                    // determine terminal height

  std::string getTaskContext (const std::string&, std::string, bool fallback=true);

  const std::vector <std::string> getColumns () const;
  void getLimits (int&, int&);

  bool color ();                       // TTY or <other>?
  bool verbose (const std::string&);   // Verbosity control

  void header (const std::string&);    // Header message sink
  void footnote (const std::string&);  // Footnote message sink
  void debug (const std::string&);     // Debug message sink
  void error (const std::string&);     // Error message sink - non-maskable

  void decomposeSortField (const std::string&, std::string&, bool&, bool&);
  void debugTiming (const std::string&, const Timer&);

  CurrentTask withCurrentTask (const Task *);
  friend class CurrentTask;

private:
  void staticInitialization ();
  void createDefaultConfig ();
  void updateXtermTitle ();
  void updateVerbosity ();
  void loadAliases ();
  void propagateDebug ();

  static Context* context;

public:
  CLI2                                cli2                {};
  std::string                         home_dir            {};
  File                                rc_file             {"~/.taskrc"};
  Path                                data_dir            {"~/.task"};
  Configuration                       config              {};
  TDB2                                tdb2                {};
  Hooks                               hooks               {};
  bool                                determine_color_use {true};
  bool                                use_color           {true};
  bool                                run_gc              {true};
  bool                                verbosity_legacy    {false};
  std::set <std::string>              verbosity           {};
  std::vector <std::string>           headers             {};
  std::vector <std::string>           footnotes           {};
  std::vector <std::string>           errors              {};
  std::vector <std::string>           debugMessages       {};
  std::map <std::string, Command*>    commands            {};
  std::map <std::string, Column*>     columns             {};
  int                                 terminal_width      {0};
  int                                 terminal_height     {0};

  Timer                               timer_total         {};
  long                                time_total_us       {0};
  long                                time_init_us        {0};
  long                                time_load_us        {0};
  long                                time_gc_us          {0};
  long                                time_filter_us      {0};
  long                                time_commit_us      {0};
  long                                time_sort_us        {0};
  long                                time_render_us      {0};
  long                                time_hooks_us       {0};

  // the current task for DOM references, or NULL if there is no task
  const Task *                        currentTask         {NULL};
};

////////////////////////////////////////////////////////////////////////////////
// CurrentTask resets Context::currentTask to previous context task on destruction; this ensures
// that this context value is restored when exiting the scope where the context was applied.
class CurrentTask {
public:
  ~CurrentTask();

private:
  CurrentTask(Context &context, const Task *previous);

  Context &context;
  const Task *previous;

  friend class Context;
};

#endif
////////////////////////////////////////////////////////////////////////////////
