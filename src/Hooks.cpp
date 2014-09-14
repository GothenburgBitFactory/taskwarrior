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

#include <cmake.h>
#include <algorithm>
// If <iostream> is included, put it after <stdio.h>, because it includes
// <stdio.h>, and therefore would ignore the _WITH_GETLINE.
#ifdef FREEBSD
#define _WITH_GETLINE
#endif
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <Context.h>
#include <Hooks.h>
#include <text.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Hooks::Hooks ()
: _enabled (true)
{
}

////////////////////////////////////////////////////////////////////////////////
Hooks::~Hooks ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::initialize ()
{
  // Scan <rc.data.location>/hooks
  Directory d (context.config.get ("data.location"));
  d += "hooks";
  if (d.is_directory () &&
      d.readable ())
  {
    _scripts = d.list ();
    std::sort (_scripts.begin (), _scripts.end ());
  }

  _enabled = context.config.getBoolean ("hooks");
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::enable (bool value)
{
  bool old_value = _enabled;
  _enabled = value;
  return old_value;
}

////////////////////////////////////////////////////////////////////////////////
// The on-launch event is triggered once, after initialization, before any
// processing occurs, i.e first
//
// Input:
// - none
//
// Output:
// - all emitted JSON lines are added/modified as tasks, if the exit code is
//   zero, otherwise ignored.
// - minimal new task:  {"description":"Buy milk"}
// - to modify a task include complete JSON
// - all emitted non-JSON lines are considered feedback messages if the exit
//   code is zero, otherwise they are considered errors.
//
void Hooks::onLaunch ()
{
  if (! _enabled)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-launch");
  std::vector <std::string>::iterator i;
  for (i = matchingScripts.begin (); i != matchingScripts.end (); ++i)
  {
    std::string output;
    std::vector <std::string> args;
    int status = execute (*i, args, "", output);

    std::vector <std::string> lines;
    split (lines, output, '\n');
    std::vector <std::string>::iterator line;

    if (status == 0)
    {
      for (line = lines.begin (); line != lines.end (); ++line)
      {
        if (line->length () && (*line)[0] == '{')
        {
          // Only 'add' is possible.
          Task newTask (*line);
          context.tdb2.add (newTask);
        }
        else
          context.header (*line);
      }
    }
    else
    {
      for (line = lines.begin (); line != lines.end (); ++line)
        if (line->length () && (*line)[0] != '{')
          context.error (*line);

      throw 0;  // This is how hooks silently terminate processing.
    }
  }

  context.timer_hooks.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// The on-exit event is triggered once, after all processing is complete, i.e.
// last
//
// Input:
// - read-only line of JSON for each task added/modified
//
// Output:
// - any emitted JSON is ignored
// - all emitted non-JSON lines are considered feedback messages if the exit
//   code is zero, otherwise they are considered errors.
//
void Hooks::onExit ()
{
  if (! _enabled)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-exit");
  std::vector <std::string>::iterator i;
  for (i = matchingScripts.begin (); i != matchingScripts.end (); ++i)
  {
    std::string input;
    std::string output;
    std::vector <std::string> args;
    int status = execute (*i, args, input, output);

    std::vector <std::string> lines;
    split (lines, output, '\n');
    std::vector <std::string>::iterator line;

    for (line = lines.begin (); line != lines.end (); ++line)
    {
      if (line->length () && (*line)[0] != '{')
      {
        if (status == 0)
          context.footnote (*line);
        else
          context.error (*line);
      }
    }
  }

  context.timer_hooks.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// The on-add event is triggered separately for each task added
//
// Input:
// - line of JSON for the task added
//
// Output:
// - all emitted JSON lines are added/modified as tasks, if the exit code is
//   zero, otherwise ignored.
// - minimal new task:  {"description":"Buy milk"}
// - to modify a task include complete JSON
// - all emitted non-JSON lines are considered feedback messages if the exit
//   code is zero, otherwise they are considered errors.
//
void Hooks::onAdd (std::vector <Task>& changes)
{
  if (! _enabled)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-add");
  std::vector <std::string>::iterator i;
  for (i = matchingScripts.begin (); i != matchingScripts.end (); ++i)
  {
    std::string input = changes[0].composeJSON () + "\n";
    std::string output;
    std::vector <std::string> args;
    int status = execute (*i, args, input, output);

    std::vector <std::string> lines;
    split (lines, output, '\n');
    std::vector <std::string>::iterator line;

    if (status == 0)
    {
      changes.clear ();
      for (line = lines.begin (); line != lines.end (); ++line)
      {
        if (line->length () && (*line)[0] == '{')
          changes.push_back (Task (*line));
        else
          context.footnote (*line);
      }
    }
    else
    {
      for (line = lines.begin (); line != lines.end (); ++line)
        if (line->length () && (*line)[0] != '{')
          context.error (*line);

      throw 0;  // This is how hooks silently terminate processing.
    }
  }

  context.timer_hooks.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// The on-modify event is triggered separately for each task added or modified
//
// Input:
// - line of JSON for the original task
// - line of JSON for the modified task, the diff being the modification
//
// Output:
// - all emitted JSON lines are added/modified as tasks, if the exit code is
//   zero, otherwise ignored.
// - minimal new task:  {"description":"Buy milk"}
// - to modify a task include complete JSON
// - all emitted non-JSON lines are considered feedback messages if the exit
//   code is zero, otherwise they are considered errors.
//
void Hooks::onModify (const Task& before, std::vector <Task>& changes)
{
  if (! _enabled)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-modify");
  std::vector <std::string>::iterator i;
  for (i = matchingScripts.begin (); i != matchingScripts.end (); ++i)
  {
    std::string afterJSON = changes[0].composeJSON ();
    std::string input = before.composeJSON ()
                      + "\n"
                      + afterJSON
                      + "\n";
    std::string output;
    std::vector <std::string> args;
    int status = execute (*i, args, input, output);

    std::vector <std::string> lines;
    split (lines, output, '\n');
    std::vector <std::string>::iterator line;

    if (status == 0)
    {
      changes.clear ();
      for (line = lines.begin (); line != lines.end (); ++line)
      {
        if (line->length () && (*line)[0] == '{')
          changes.push_back (Task (*line));
        else
          context.footnote (*line);
      }
    }
    else
    {
      for (line = lines.begin (); line != lines.end (); ++line)
        if (line->length () && (*line)[0] != '{')
          context.error (*line);

      throw 0;  // This is how hooks silently terminate processing.
    }
  }

  context.timer_hooks.stop ();
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Hooks::list ()
{
  return _scripts;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Hooks::scripts (const std::string& event)
{
  std::vector <std::string> matching;
  std::vector <std::string>::iterator i;
  for (i = _scripts.begin (); i != _scripts.end (); ++i)
  {
    if (i->find ("/" + event) != std::string::npos)
    {
      File script (*i);
      if (script.executable ())
        matching.push_back (*i);
    }
  }

  return matching;
}

////////////////////////////////////////////////////////////////////////////////
