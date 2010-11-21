////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
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
#include <algorithm>
#include "Context.h"
#include "Hooks.h"
#include "Timer.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Hook::Hook ()
: event ("")
, file ("")
, function ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Hook::Hook (const std::string& e, const std::string& f, const std::string& fn)
: event (e)
, file (f)
, function (fn)
{
}

////////////////////////////////////////////////////////////////////////////////
Hook::Hook (const Hook& other)
{
  event = other.event;
  file = other.file;
  function = other.function;
}

////////////////////////////////////////////////////////////////////////////////
Hook& Hook::operator= (const Hook& other)
{
  if (this != &other)
  {
    event = other.event;
    file = other.file;
    function = other.function;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Hooks::Hooks ()
{
  validProgramEvents.push_back ("post-start");
  validProgramEvents.push_back ("post-commit");
  validProgramEvents.push_back ("pre-fatal-error");
  validProgramEvents.push_back ("pre-exit");
  validProgramEvents.push_back ("pre-command-line");
  validProgramEvents.push_back ("post-command-line");
  validProgramEvents.push_back ("pre-command-line-override");
  validProgramEvents.push_back ("post-command-line-override");
  validProgramEvents.push_back ("pre-config-create");
  validProgramEvents.push_back ("post-config-create");
  validProgramEvents.push_back ("pre-config-read");
  validProgramEvents.push_back ("post-config-read");
  validProgramEvents.push_back ("pre-config-value-read");
  validProgramEvents.push_back ("post-config-value-read");
  validProgramEvents.push_back ("pre-config-value-write");
  validProgramEvents.push_back ("post-config-value-write");
  validProgramEvents.push_back ("pre-file-lock");
  validProgramEvents.push_back ("post-file-lock");
  validProgramEvents.push_back ("pre-file-unlock");
  validProgramEvents.push_back ("post-file-unlock");
  validProgramEvents.push_back ("pre-file-read");
  validProgramEvents.push_back ("post-file-read");
  validProgramEvents.push_back ("pre-file-write");
  validProgramEvents.push_back ("post-file-write");
  validProgramEvents.push_back ("pre-output");
  validProgramEvents.push_back ("post-output");
  validProgramEvents.push_back ("pre-debug");
  validProgramEvents.push_back ("post-debug");
  validProgramEvents.push_back ("pre-header");
  validProgramEvents.push_back ("post-header");
  validProgramEvents.push_back ("pre-footnote");
  validProgramEvents.push_back ("post-footnote");
  validProgramEvents.push_back ("pre-dispatch");
  validProgramEvents.push_back ("post-dispatch");
  validProgramEvents.push_back ("pre-gc");
  validProgramEvents.push_back ("post-gc");
  validProgramEvents.push_back ("pre-archive");
  validProgramEvents.push_back ("post-archive");
  validProgramEvents.push_back ("pre-purge");
  validProgramEvents.push_back ("post-purge");
  validProgramEvents.push_back ("pre-recurrence");
  validProgramEvents.push_back ("post-recurrence");
  validProgramEvents.push_back ("pre-interactive");
  validProgramEvents.push_back ("post-interactive");
  validProgramEvents.push_back ("pre-undo");
  validProgramEvents.push_back ("post-undo");
  validProgramEvents.push_back ("pre-confirm");
  validProgramEvents.push_back ("post-confirm");
  validProgramEvents.push_back ("pre-shell-prompt");
  validProgramEvents.push_back ("post-shell-prompt");
  validProgramEvents.push_back ("pre-add-command");
  validProgramEvents.push_back ("post-add-command");
  validProgramEvents.push_back ("pre-annotate-command");
  validProgramEvents.push_back ("post-annotate-command");
  validProgramEvents.push_back ("pre-denotate-command");
  validProgramEvents.push_back ("post-denotate-command");
  validProgramEvents.push_back ("pre-append-command");
  validProgramEvents.push_back ("post-append-command");
  validProgramEvents.push_back ("pre-calendar-command");
  validProgramEvents.push_back ("post-calendar-command");
  validProgramEvents.push_back ("pre-color-command");
  validProgramEvents.push_back ("post-color-command");
  validProgramEvents.push_back ("pre-config-command");
  validProgramEvents.push_back ("post-config-command");
  validProgramEvents.push_back ("pre-custom-report-command");
  validProgramEvents.push_back ("post-custom-report-command");
  validProgramEvents.push_back ("pre-default-command");
  validProgramEvents.push_back ("post-default-command");
  validProgramEvents.push_back ("pre-delete-command");
  validProgramEvents.push_back ("post-delete-command");
  validProgramEvents.push_back ("pre-done-command");
  validProgramEvents.push_back ("post-done-command");
  validProgramEvents.push_back ("pre-duplicate-command");
  validProgramEvents.push_back ("post-duplicate-command");
  validProgramEvents.push_back ("pre-edit-command");
  validProgramEvents.push_back ("post-edit-command");
  validProgramEvents.push_back ("pre-export-command");
  validProgramEvents.push_back ("post-export-command");
  validProgramEvents.push_back ("pre-ghistory-command");
  validProgramEvents.push_back ("post-ghistory-command");
  validProgramEvents.push_back ("pre-history-command");
  validProgramEvents.push_back ("post-history-command");
  validProgramEvents.push_back ("pre-burndown-command");
  validProgramEvents.push_back ("post-burndown-command");
  validProgramEvents.push_back ("pre-import-command");
  validProgramEvents.push_back ("post-import-command");
  validProgramEvents.push_back ("pre-info-command");
  validProgramEvents.push_back ("post-info-command");
	validProgramEvents.push_back ("pre-merge-command");
	validProgramEvents.push_back ("post-merge-command");
  validProgramEvents.push_back ("pre-modify-command");
  validProgramEvents.push_back ("post-modify-command");
  validProgramEvents.push_back ("pre-prepend-command");
  validProgramEvents.push_back ("post-prepend-command");
  validProgramEvents.push_back ("pre-projects-command");
  validProgramEvents.push_back ("post-projects-command");
	validProgramEvents.push_back ("pre-pull-command");
	validProgramEvents.push_back ("post-pull-command");
	validProgramEvents.push_back ("pre-push-command");
	validProgramEvents.push_back ("post-push-command");
  validProgramEvents.push_back ("pre-shell-command");
  validProgramEvents.push_back ("post-shell-command");
  validProgramEvents.push_back ("pre-start-command");
  validProgramEvents.push_back ("post-start-command");
  validProgramEvents.push_back ("pre-stats-command");
  validProgramEvents.push_back ("post-stats-command");
  validProgramEvents.push_back ("pre-stop-command");
  validProgramEvents.push_back ("post-stop-command");
  validProgramEvents.push_back ("pre-summary-command");
  validProgramEvents.push_back ("post-summary-command");
  validProgramEvents.push_back ("pre-tags-command");
  validProgramEvents.push_back ("post-tags-command");
  validProgramEvents.push_back ("pre-timesheet-command");
  validProgramEvents.push_back ("post-timesheet-command");
  validProgramEvents.push_back ("pre-undo-command");
  validProgramEvents.push_back ("post-undo-command");
  validProgramEvents.push_back ("pre-usage-command");
  validProgramEvents.push_back ("post-usage-command");
  validProgramEvents.push_back ("pre-version-command");
  validProgramEvents.push_back ("post-version-command");

  validListEvents.push_back ("pre-filter");
  validListEvents.push_back ("post-filter");

  validTaskEvents.push_back ("pre-display");
  validTaskEvents.push_back ("pre-modification");
  validTaskEvents.push_back ("post-modification");
  validTaskEvents.push_back ("pre-delete");
  validTaskEvents.push_back ("post-delete");
  validTaskEvents.push_back ("pre-add");
  validTaskEvents.push_back ("post-add");
  validTaskEvents.push_back ("pre-undo");
  validTaskEvents.push_back ("post-undo");
  validTaskEvents.push_back ("pre-wait");
  validTaskEvents.push_back ("post-wait");
  validTaskEvents.push_back ("pre-unwait");
  validTaskEvents.push_back ("post-unwait");
  validTaskEvents.push_back ("pre-completed");
  validTaskEvents.push_back ("post-completed");
  validTaskEvents.push_back ("pre-priority-change");
  validTaskEvents.push_back ("post-priority-change");
  validTaskEvents.push_back ("pre-project-change");
  validTaskEvents.push_back ("post-project-change");
  validTaskEvents.push_back ("pre-substitution");
  validTaskEvents.push_back ("post-substitution");
  validTaskEvents.push_back ("pre-annotation");
  validTaskEvents.push_back ("post-annotation");
  validTaskEvents.push_back ("pre-tag");
  validTaskEvents.push_back ("post-tag");
  validTaskEvents.push_back ("pre-detag");
  validTaskEvents.push_back ("post-detag");
  validTaskEvents.push_back ("pre-colorization");
  validTaskEvents.push_back ("post-colorization");

  validFieldEvents.push_back ("format-number");
  validFieldEvents.push_back ("format-date");
  validFieldEvents.push_back ("format-duration");
  validFieldEvents.push_back ("format-text");
  validFieldEvents.push_back ("format-id");
  validFieldEvents.push_back ("format-uuid");
  validFieldEvents.push_back ("format-project");
  validFieldEvents.push_back ("format-priority");
  validFieldEvents.push_back ("format-priority_long");
  validFieldEvents.push_back ("format-entry");
  validFieldEvents.push_back ("format-start");
  validFieldEvents.push_back ("format-end");
  validFieldEvents.push_back ("format-due");
  validFieldEvents.push_back ("format-countdown");
  validFieldEvents.push_back ("format-countdown_compact");
  validFieldEvents.push_back ("format-age");
  validFieldEvents.push_back ("format-age_compact");
  validFieldEvents.push_back ("format-active");
  validFieldEvents.push_back ("format-tags");
  validFieldEvents.push_back ("format-recur");
  validFieldEvents.push_back ("format-recurrence_indicator");
  validFieldEvents.push_back ("format-tag_indicator");
  validFieldEvents.push_back ("format-description_only");
  validFieldEvents.push_back ("format-description");
  validFieldEvents.push_back ("format-wait");
  validFieldEvents.push_back ("format-prompt");
  validFieldEvents.push_back ("format-depends");
}

////////////////////////////////////////////////////////////////////////////////
Hooks::~Hooks ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Enumerate all hooks, and tell API about the script files it must load in
// order to call them.  Note that API will perform a deferred read, which means
// that if it isn't called, a script will not be loaded.
void Hooks::initialize ()
{
#ifdef HAVE_LIBLUA
  api.initialize ();
#endif

  // Allow a master switch to turn the whole thing off.
  bool big_red_switch = context.config.getBoolean ("hooks");
  if (big_red_switch)
  {
    std::vector <std::string> vars;
    context.config.all (vars);

    std::vector <std::string>::iterator it;
    for (it = vars.begin (); it != vars.end (); ++it)
    {
      std::string type;
      std::string name;
      std::string value;

      // "<type>.<name>"
      Nibbler n (*it);
      if (n.getUntil ('.', type) &&
          type == "hook"         &&
          n.skip ('.')           &&
          n.getUntilEOS (name))
      {
        std::string value = context.config.get (*it);
        Nibbler n (value);

        // <path>:<function> [, ...]
        while (!n.depleted ())
        {
          std::string file;
          std::string function;
          if (n.getUntil (':', file) &&
              n.skip (':')           &&
              n.getUntil (',', function))
          {
            context.debug (std::string ("Event '") + name + "' hooked by " + file + ", function " + function);
            Hook h (name, Path::expand (file), function);
            all.push_back (h);

            (void) n.skip (',');
          }
          else
            throw std::string ("Malformed hook definition '") + *it + "'.";
        }
      }
    }
  }
  else
    context.debug ("Hooks::initialize - hook system off");
}

////////////////////////////////////////////////////////////////////////////////
// Program hooks.
bool Hooks::trigger (const std::string& event)
{
#ifdef HAVE_LIBLUA
  std::vector <Hook>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->event == event)
    {
      Timer timer (std::string ("Hooks::trigger ") + event);

      if (validProgramEvent (event))
      {
        context.debug (std::string ("Event ") + event + " triggered");
        if (! api.callProgramHook (it->file, it->function))
          return false;
      }
      else
        throw std::string ("Unrecognized hook event '") + event + "'.";
    }
  }
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// List hooks.
bool Hooks::trigger (const std::string& event, std::vector <Task>& tasks)
{
#ifdef HAVE_LIBLUA
  std::vector <Hook>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->event == event)
    {
      Timer timer (std::string ("Hooks::trigger ") + event);

      if (validListEvent (event))
      {
        context.debug (std::string ("Event ") + event + " triggered");
        if (! api.callListHook (it->file, it->function, tasks))
          return false;
      }
      else
        throw std::string ("Unrecognized hook event '") + event + "'.";
    }
  }
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Task hooks.
bool Hooks::trigger (const std::string& event, Task& task)
{
#ifdef HAVE_LIBLUA
  std::vector <Hook>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->event == event)
    {
      Timer timer (std::string ("Hooks::trigger ") + event);

      if (validTaskEvent (event))
      {
        context.debug (std::string ("Event ") + event + " triggered");
        if (! api.callTaskHook (it->file, it->function, task))
          return false;
      }
      else
        throw std::string ("Unrecognized hook event '") + event + "'.";
    }
  }
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Field hooks.
bool Hooks::trigger (
  const std::string& event,
  const std::string& name,
  std::string& value)
{
#ifdef HAVE_LIBLUA
  std::vector <Hook>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->event == event)
    {
      Timer timer (std::string ("Hooks::trigger ") + event);

      if (validFieldEvent (event))
      {
        context.debug (std::string ("Event ") + event + " triggered");
        if (! api.callFieldHook (it->file, it->function, name, value))
          return false;
      }
      else
        throw std::string ("Unrecognized hook event '") + event + "'.";
    }
  }
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::validProgramEvent (const std::string& event)
{
  if (std::find (validProgramEvents.begin (), validProgramEvents.end (), event) != validProgramEvents.end ())
    return true;

  return false;
}

bool Hooks::validListEvent (const std::string& event)
{
  if (std::find (validListEvents.begin (), validListEvents.end (), event) != validListEvents.end ())
    return true;

  return false;
}

bool Hooks::validTaskEvent (const std::string& event)
{
  if (std::find (validTaskEvents.begin (), validTaskEvents.end (), event) != validTaskEvents.end ())
    return true;

  return false;
}

bool Hooks::validFieldEvent (const std::string& event)
{
  if (std::find (validFieldEvents.begin (), validFieldEvents.end (), event) != validFieldEvents.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
