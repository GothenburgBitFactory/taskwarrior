////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <JSON.h>
#include <Hooks.h>
#include <text.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Hooks::Hooks ()
: _enabled (true)
, _debug (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Hooks::~Hooks ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::initialize ()
{
  _debug = context.config.getInteger ("debug.hooks");

  // Scan <rc.data.location>/hooks
  Directory d (context.config.get ("data.location"));
  d += "hooks";
  if (d.is_directory () &&
      d.readable ())
  {
    _scripts = d.list ();
    std::sort (_scripts.begin (), _scripts.end ());

    if (_debug >= 1)
    {
      std::vector <std::string>::iterator i;
      for (i = _scripts.begin (); i != _scripts.end (); ++i)
        context.debug ("Found hook script " + *i);
    }
  }
  else if (_debug >= 1)
    context.debug ("Hook directory not readable: " + d._data);

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
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onLaunch ()
{
  if (! _enabled)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-launch");
  if (matchingScripts.size ())
  {
    std::vector <std::string>::iterator script;
    for (script = matchingScripts.begin (); script != matchingScripts.end (); ++script)
    {
      std::vector <std::string> input;
      std::vector <std::string> output;
      int status = callHookScript (*script, input, output);

      std::vector <std::string>::iterator line;
      for (line = output.begin (); line != output.end (); ++line)
      {
        if (isJSON (*line))
        {
          if (status == 0)
          {
            // Only 'add' is possible.
            Task newTask (*line);
            context.tdb2.add (newTask);
          }
        }
        else
        {
          if (status == 0)
            context.header (*line);
          else
            context.error (*line);
        }
      }

      if (status)
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
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onExit ()
{
  if (! _enabled)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-exit");
  if (matchingScripts.size ())
  {
    // Get the set of changed tasks.
    std::vector <Task> tasks;
    context.tdb2.get_changes (tasks);

    // Convert to a vector of strings.
    std::vector <std::string> input;
    std::vector <Task>::const_iterator t;
    for (t = tasks.begin (); t != tasks.end (); ++t)
      input.push_back (t->composeJSON ());

    // Call the hook scripts, with the invariant input.
    std::vector <std::string>::iterator script;
    for (script = matchingScripts.begin (); script != matchingScripts.end (); ++script)
    {
      std::vector <std::string> output;
      int status = callHookScript (*script, input, output);

      std::vector <std::string>::iterator line;
      for (line = output.begin (); line != output.end (); ++line)
      {
        if (isJSON (*line))
        {
          context.error ("JSON output ignored: {1}");
        }
        else
        {
          if (status == 0)
            context.footnote (*line);
          else
            context.error (*line);
        }
      }

      if (status)
        throw 0;  // This is how hooks silently terminate processing.
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
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onAdd (std::vector <Task>& tasks)
{
  if (! _enabled || tasks.size () < 1)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-add");
  if (matchingScripts.size ())
  {
    // Convert vector of tasks to a vector of strings.
    std::vector <std::string> input;
    input.push_back (tasks[0].composeJSON ());

    // Call the hook scripts.
    std::vector <std::string>::iterator script;
    for (script = matchingScripts.begin (); script != matchingScripts.end (); ++script)
    {
      std::vector <std::string> output;
      int status = callHookScript (*script, input, output);

      input.clear ();
      std::vector <std::string>::iterator line;
      for (line = output.begin (); line != output.end (); ++line)
      {
        if (isJSON (*line))
        {
          if (status == 0)
            input.push_back (*line);
        }
        else
        {
          if (status == 0)
            context.footnote (*line);
          else
            context.error (*line);
        }
      }

      if (status)
        throw 0;  // This is how hooks silently terminate processing.
    }

    // Transfer the modified task lines back to the original task list.
    tasks.clear ();
    std::vector <std::string>::iterator i;
    for (i = input.begin (); i != input.end (); ++i)
      tasks.push_back (Task (*i));
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
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onModify (const Task& before, std::vector <Task>& tasks)
{
  if (! _enabled || tasks.size () < 1)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-modify");
  if (matchingScripts.size ())
  {
    // Prepare invariants.
    std::string beforeJSON = before.composeJSON ();
    std::string uuidPattern = "\"uuid\":\"" + before.get ("uuid") + "\"";

    // Convert vector of tasks to a vector of strings.
    std::vector <std::string> input;
    input.push_back (beforeJSON);               // [0] original, never changes
    input.push_back (tasks[0].composeJSON ());  // [1] original'

    // Call the hook scripts.
    std::vector <std::string>::iterator script;
    for (script = matchingScripts.begin (); script != matchingScripts.end (); ++script)
    {
      std::vector <std::string> firstTwoOnly;
      firstTwoOnly.push_back (input[0]);
      firstTwoOnly.push_back (input[1]);

      std::vector <std::string> output;
      int status = callHookScript (*script, firstTwoOnly, output);

      // Start from scratch.
      input[1] = "";                            // [1] placeholder for original'

      std::vector <std::string>::iterator line;
      for (line = output.begin (); line != output.end (); ++line)
      {
        if (isJSON (*line))
        {
          if (status == 0)
          {
            if (line->find (uuidPattern) != std::string::npos)
              input[1] = *line;                 // [1] original'
            else
              input.push_back (*line);          // [n > 1] extras
          }
        }
        else
        {
          if (status == 0)
              context.footnote (*line);
        else
            context.error (*line);
        }
      }

      if (status)
        throw 0;  // This is how hooks silently terminate processing.
    }

    // Transfer the modified task lines back to the original task list.
    tasks.clear ();
    std::vector <std::string>::iterator i;
    for (i = input.begin (); i != input.end (); ++i)
      if (i != input.begin ())
        tasks.push_back (Task (*i));
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
bool Hooks::isJSON (const std::string& input) const
{
  // Does it even look like JSON?  {...}
  if (input.length () > 2 &&
      input[0] == '{'     &&
      input[input.length () - 1] == '}')
  {
    try
    {
      // The absolute minimum a task needs is:
      bool foundDescription = false;

      // Parse the whole thing.
      json::value* root = json::parse (input);
      if (root->type () == json::j_object)
      {
        json::object* root_obj = (json::object*)root;

        // For each object element...
        json_object_iter i;
        for (i  = root_obj->_data.begin ();
             i != root_obj->_data.end ();
             ++i)
        {
          // If the attribute is a recognized column.
          std::string type = Task::attributes[i->first];
          if (type == "string" && i->first == "description")
            foundDescription = true;
        }
      }
      else
        throw std::string ("Object expected.");

      // It's JSON, but is it a task?
      if (! foundDescription)
        throw std::string ("Missing 'description' attribute, of type 'string'.");

      // Yep, looks like a JSON task.
      return true;
    }

    catch (const std::string& e)
    {
      if (_debug >= 1)
        context.error ("Hook output looks like JSON, but is not a valid task.");

      if (_debug >= 2)
        context.error ("JSON " + e);
    }

    catch (...)
    {
      if (_debug >= 1)
        context.error ("Hook output looks like JSON, but fails to parse.");
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
int Hooks::callHookScript (
  const std::string& script,
  const std::vector <std::string>& input,
  std::vector <std::string>& output)
{
  if (_debug >= 1)
    context.debug ("Hooks: Calling " + script);

  if (_debug >= 2)
  {
    context.debug ("Hooks: input");
    std::vector <std::string>::const_iterator i;
    for (i = input.begin (); i != input.end (); ++i)
      context.debug ("  " + *i);
  }

  std::string inputStr;
  std::vector <std::string>::const_iterator i;
  for (i = input.begin (); i != input.end (); ++i)
    inputStr += *i + "\n";

  std::string outputStr;
  std::vector <std::string> args;
  int status = execute (script, args, inputStr, outputStr);

  split (output, outputStr, '\n');

  if (_debug >= 2)
  {
    context.debug ("Hooks: output");
    std::vector <std::string>::iterator i;
    for (i = output.begin (); i != output.end (); ++i)
      if (*i != "")
        context.debug ("  " + *i);

    context.debug (format ("Hooks: Completed with status {1}", status));
    context.debug (" "); // Blank line
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
