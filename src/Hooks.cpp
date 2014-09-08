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
}

////////////////////////////////////////////////////////////////////////////////
// The on-launch event is triggered once, after initialization, before an
// processing occurs
//
// No input
//
// Output:
// - all emitted JSON lines must be fully-formed tasks
// - all emitted non-JSON lines are considered feedback messages
//
// Exit:
//   0 Means:     - all emitted JSON lines are added/modified
//                - all emitted non-JSON lines become footnote entries
//   non-0 Means: - all emitted JSON lines are ignored
//                - all emitted non-JSON lines become error entries
//
void Hooks::onLaunch ()
{
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
// Input:
// - A read-only line of JSON for each task added/modified
//
// Output:
// - all emitted non-JSON lines are considered feedback messages
// - The onExit event occurs too late to allow any changes, so the input is not
//   to be modified
//
// Exit:
//   0 Means:     - all emitted non-JSON lines become footnote entries
//   non-0 Means: - all emitted non-JSON lines become error entries
bool Hooks::onExit ()
{
  context.timer_hooks.start ();

  // Status indicates whether hooks resulted in new/modified tasks.
  bool status = false;

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
  return status;
}

////////////////////////////////////////////////////////////////////////////////
// The on-add event is triggered separately for each task added
//
// Input:
// - A line of JSON for the task added
//
// Output:
// - all emitted JSON lines must be fully-formed tasks
// - all emitted non-JSON lines are considered feedback messages
//
// Exit:
//   0 Means:     - all emitted JSON lines are added/modified
//                - all emitted non-JSON lines become footnote entries
//   non-0 Means: - all emitted JSON lines are ignored
//                - all emitted non-JSON lines become error entries
//
void Hooks::onAdd (Task& after)
{
  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-add");
  std::vector <std::string>::iterator i;
  for (i = matchingScripts.begin (); i != matchingScripts.end (); ++i)
  {
    std::string input = after.composeJSON () + "\n";
    std::string output;
    std::vector <std::string> args;
    int status = execute (*i, args, input, output);

    std::vector <std::string> lines;
    split (lines, output, '\n');
    std::vector <std::string>::iterator line;

    if (status == 0)
    {
      bool first = true;
      for (line = lines.begin (); line != lines.end (); ++line)
      {
        if (line->length () && (*line)[0] == '{')
        {
          Task newTask (*line);

          if (first)
          {
            after = newTask;
            first = false;
          }
          else
            context.tdb2.add (newTask);
        }
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
// - A line of JSON for the original task
// - A line of JSON for the modified task
//
// Output:
// - all emitted JSON lines must be fully-formed tasks
// - all emitted non-JSON lines are considered feedback messages
//
// Exit:
//   0 Means:     - all emitted JSON lines are added/modified
//                - all emitted non-JSON lines become footnote entries
//   non-0 Means: - all emitted JSON lines are ignored
//                - all emitted non-JSON lines become error entries
void Hooks::onModify (const Task& before, Task& after)
{
  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-modify");
  std::vector <std::string>::iterator i;
  for (i = matchingScripts.begin (); i != matchingScripts.end (); ++i)
  {
    std::string afterJSON = after.composeJSON ();
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
      bool first = true;
      for (line = lines.begin (); line != lines.end (); ++line)
      {
        if (line->length () && (*line)[0] == '{')
        {
          Task newTask (*line);

          if (first)
          {
            after = newTask;
            first = false;
          }
          else
            context.tdb2.add (newTask);
        }
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
