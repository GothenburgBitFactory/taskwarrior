////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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

#include <iostream>
#include <sstream>
#include <string>
#include "Context.h"
#include "main.h"
#include "log.h"
#include "Layout.h"
#include "i18n.h"
#include "../auto.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

////////////////////////////////////////////////////////////////////////////////
int Context::handleInteractive ()
{
#ifdef HAVE_LIBNCURSES
  // TODO Need to wrap all UI code with HAVE_LIBNCURSES.

  // TODO Load this, and all others from .taskrc to override the defaults.
  logSetDirectory (".");
  logSetName ("task");
  logDuplicates (false);
  logEnable (false);

  try
  {
    // TODO Ick.  Clean this up.
    std::string prefix = " (vertical (horizontal 2 (panel title *) (panel stats 22)) (horizontal 1 (panel message 60%) (panel clock *)) ";
    std::string suffix = " (panel keys 2)))";

    std::string layout_def          = "(layout default"      + prefix + "(panel report *)"       + suffix;
    std::string layout_report_stats = "(layout report.stats" + prefix + "(panel report.stats *)" + suffix;

    UI ui;              // Construct a UI coordinator.
    createLayout (ui, "default",      layout_def);
    createLayout (ui, "report.stats", layout_report_stats);

    ui.interactive ();  // Start ncurses, event loop.
  }

  // TODO Integrate regular task error handling, using Context::debug.
  catch (int e)          { std::cout << e << "\n"; }
  catch (const char* e)  { std::cout << e << "\n"; }
  catch (std::string& e) { std::cout << e << "\n"; }
  catch (...)            { std::cout << "Unknown error.\n"; }

  logWrite ("---");

#else

  throw stringtable.get (INTERACTIVE_NO_NCURSES,
                         "Interactive task is only available when built with ncurses "
                         "support.");

#endif

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Context::createLayout (
  UI& ui,
  const std::string& name,
  const std::string& def)
{
  // A layout named 'foo' can be overridden with a .taskrc config variable that
  // is named 'layout.foo'.
  std::string definition = config.get ("layout." + name);
  if (definition == "")
    definition = def;

  Layout* l = new Layout (definition);
  ui.add (name, l);
}

////////////////////////////////////////////////////////////////////////////////
int Context::getWidth ()
{
  // Determine window size, and set table accordingly.
  int width = config.getInteger ("defaultwidth");

#ifdef HAVE_LIBNCURSES
  if (config.getBoolean ("curses"))
  {
    initscr ();
    width = COLS;
    endwin ();

    std::stringstream out;
    out << "Context::getWidth: ncurses determined width of " << width << " characters";
    debug (out.str ());
  }
  else
    debug ("Context::getWidth: ncurses available but disabled.");
#else
  std::stringstream out;
  out << "Context::getWidth: no ncurses, using width of " << width << " characters";
  debug (out.str ());
#endif

  return width;
}

////////////////////////////////////////////////////////////////////////////////
