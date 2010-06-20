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
            throw std::string ("Malformed hook definition '") + *it + "'";
        }
      }
    }
  }
  else
    context.debug ("Hooks::initialize - hook system shut off");
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
        throw std::string ("Unrecognized hook event '") + event + "'";
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
        throw std::string ("Unrecognized hook event '") + event + "'";
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
        throw std::string ("Unrecognized hook event '") + event + "'";
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
        throw std::string ("Unrecognized hook event '") + event + "'";
    }
  }
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::validProgramEvent (const std::string& event)
{
  if (event == "post-start"                ||
      event == "pre-fatal-error"           ||
      event == "pre-exit"                  ||
      event == "pre-command-line"          || event == "post-command-line"          ||
      event == "pre-command-line-override" || event == "post-command-line-override" ||
      event == "pre-config-create"         || event == "post-config-create"         ||
      event == "pre-config-read"           || event == "post-config-read"           ||
      event == "pre-config-value-read"     || event == "post-config-value-read"     ||
      event == "pre-config-value-write"    || event == "post-config-value-write"    ||
      event == "pre-file-lock"             || event == "post-file-lock"             ||
      event == "pre-file-unlock"           || event == "post-file-unlock"           ||
      event == "pre-file-read"             || event == "post-file-read"             ||
      event == "pre-file-write"            || event == "post-file-write"            ||
      event == "pre-output"                || event == "post-output"                ||
      event == "pre-debug"                 || event == "post-debug"                 ||
      event == "pre-header"                || event == "post-header"                ||
      event == "pre-footnote"              || event == "post-footnote"              ||
      event == "pre-dispatch"              || event == "post-dispatch"              ||
      event == "pre-gc"                    || event == "post-gc"                    ||
      event == "pre-archive"               || event == "post-archive"               ||
      event == "pre-purge"                 || event == "post-purge"                 ||
      event == "pre-recurrence"            || event == "post-recurrence"            ||
      event == "pre-interactive"           || event == "post-interactive"           ||
      event == "pre-undo"                  || event == "post-undo"                  ||
      event == "pre-confirm"               || event == "post-confirm"               ||
      event == "pre-shell-prompt"          || event == "post-shell-prompt"          ||
      event == "pre-add-command"           || event == "post-add-command"           ||
      event == "pre-annotate-command"      || event == "post-annotate-command"      ||
      event == "pre-denotate-command"      || event == "post-denotate-command"      ||
      event == "pre-append-command"        || event == "post-append-command"        ||
      event == "pre-calendar-command"      || event == "post-calendar-command"      ||
      event == "pre-color-command"         || event == "post-color-command"         ||
      event == "pre-config-command"        || event == "post-config-command"        ||
      event == "pre-custom-report-command" || event == "post-custom-report-command" ||
      event == "pre-default-command"       || event == "post-default-command"       ||
      event == "pre-delete-command"        || event == "post-delete-command"        ||
      event == "pre-done-command"          || event == "post-done-command"          ||
      event == "pre-duplicate-command"     || event == "post-duplicate-command"     ||
      event == "pre-edit-command"          || event == "post-edit-command"          ||
      event == "pre-export-command"        || event == "post-export-command"        ||
      event == "pre-ghistory-command"      || event == "post-ghistory-command"      ||
      event == "pre-history-command"       || event == "post-history-command"       ||
      event == "pre-import-command"        || event == "post-import-command"        ||
      event == "pre-info-command"          || event == "post-info-command"          ||
      event == "pre-prepend-command"       || event == "post-prepend-command"       ||
      event == "pre-projects-command"      || event == "post-projects-command"      ||
      event == "pre-shell-command"         || event == "post-shell-command"         ||
      event == "pre-start-command"         || event == "post-start-command"         ||
      event == "pre-stats-command"         || event == "post-stats-command"         ||
      event == "pre-stop-command"          || event == "post-stop-command"          ||
      event == "pre-summary-command"       || event == "post-summary-command"       ||
      event == "pre-tags-command"          || event == "post-tags-command"          ||
      event == "pre-timesheet-command"     || event == "post-timesheet-command"     ||
      event == "pre-undo-command"          || event == "post-undo-command"          ||
      event == "pre-usage-command"         || event == "post-usage-command"         ||
      event == "pre-version-command"       || event == "post-version-command")
    return true;

  return false;
}

bool Hooks::validListEvent (const std::string& event)
{
  if (event == "pre-filter" || event == "post-filter")
    return true;

  return false;
}

bool Hooks::validTaskEvent (const std::string& event)
{
  if (event == "pre-display"         ||
      event == "pre-modification"    || event == "post-modification"    ||
      event == "pre-delete"          || event == "post-delete"          ||
      event == "pre-add"             || event == "post-add"             ||
      event == "pre-undo"            || event == "post-undo"            ||
      event == "pre-wait"            || event == "post-wait"            ||
      event == "pre-unwait"          || event == "post-unwait"          ||
      event == "pre-completed"       || event == "post-completed"       ||
      event == "pre-priority-change" || event == "post-priority-change" ||
      event == "pre-project-change"  || event == "post-project-change"  ||
      event == "pre-substitution"    || event == "post-substitution"    ||
      event == "pre-annotation"      || event == "post-annotation"      ||
      event == "pre-tag"             || event == "post-tag"             ||
      event == "pre-detag"           || event == "post-detag"           ||
      event == "pre-colorization"    || event == "post-colorization")
    return true;

  return false;
}

bool Hooks::validFieldEvent (const std::string& event)
{
  if (
      event == "format-number"               ||
      event == "format-date"                 ||
      event == "format-duration"             ||
      event == "format-text"                 ||
      event == "format-id"                   ||
      event == "format-uuid"                 ||
      event == "format-project"              ||
      event == "format-priority"             ||
      event == "format-priority_long"        ||
      event == "format-entry"                ||
      event == "format-entry_time"           ||
      event == "format-start"                ||
      event == "format-start_time"           ||
      event == "format-end"                  ||
      event == "format-end_time"             ||
      event == "format-due"                  ||
      event == "format-countdown"            ||
      event == "format-countdown_compact"    ||
      event == "format-age"                  ||
      event == "format-age_compact"          ||
      event == "format-active"               ||
      event == "format-tags"                 ||
      event == "format-recur"                ||
      event == "format-recurrence_indicator" ||
      event == "format-tag_indicator"        ||
      event == "format-description_only"     ||
      event == "format-description"          ||
      event == "format-wait"                 ||
      event == "format-prompt")
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
