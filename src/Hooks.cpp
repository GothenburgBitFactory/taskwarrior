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
#include <Timer.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

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
      {
        Path p (*i);
        std::string name = p.name ();
        if (name.substr (0, 6) == "on-add"    ||
            name.substr (0, 9) == "on-modify" ||
            name.substr (0, 9) == "on-launch" ||
            name.substr (0, 7) == "on-exit")
          context.debug ("Found hook script " + *i);
        else
          context.debug ("Found misnamed hook script " + *i);
      }
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
// - JSON not allowed.
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

      std::vector <std::string> outputJSON;
      std::vector <std::string> outputFeedback;
      separateOutput (output, outputJSON, outputFeedback);

      assertNTasks (outputJSON, 0);

      if (status == 0)
      {
        std::vector <std::string>::iterator message;
        for (message = outputFeedback.begin (); message != outputFeedback.end (); ++message)
          context.footnote (*message);
      }
      else
      {
        assertFeedback (outputFeedback);

        std::vector <std::string>::iterator message;
        for (message = outputFeedback.begin (); message != outputFeedback.end (); ++message)
          context.error (*message);

        throw 0;  // This is how hooks silently terminate processing.
      }
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
// - all emitted JSON is ignored
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

      std::vector <std::string> outputJSON;
      std::vector <std::string> outputFeedback;
      separateOutput (output, outputJSON, outputFeedback);

      assertNTasks (outputJSON, 0);

      if (status == 0)
      {
        std::vector <std::string>::iterator message;
        for (message = outputFeedback.begin (); message != outputFeedback.end (); ++message)
          context.footnote (*message);
      }
      else
      {
        assertFeedback (outputFeedback);

        std::vector <std::string>::iterator message;
        for (message = outputFeedback.begin (); message != outputFeedback.end (); ++message)
          context.error (*message);

        throw 0;  // This is how hooks silently terminate processing.
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
// - emitted JSON for the input task is added, if the exit code is zero,
//   otherwise ignored.
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onAdd (Task& task)
{
  if (! _enabled)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-add");
  if (matchingScripts.size ())
  {
    // Convert task to a vector of strings.
    std::vector <std::string> input;
    input.push_back (task.composeJSON ());

    // Call the hook scripts.
    std::vector <std::string>::iterator script;
    for (script = matchingScripts.begin (); script != matchingScripts.end (); ++script)
    {
      std::vector <std::string> output;
      int status = callHookScript (*script, input, output);

      std::vector <std::string> outputJSON;
      std::vector <std::string> outputFeedback;
      separateOutput (output, outputJSON, outputFeedback);

      if (status == 0)
      {
        assertNTasks    (outputJSON, 1);
        assertValidJSON (outputJSON);
        assertSameTask  (outputJSON, task);

        // Propagate forward to the next script.
        input[0] = outputJSON[0];

        std::vector <std::string>::iterator message;
        for (message = outputFeedback.begin (); message != outputFeedback.end (); ++message)
          context.footnote (*message);
      }
      else
      {
        assertFeedback (outputFeedback);

        std::vector <std::string>::iterator message;
        for (message = outputFeedback.begin (); message != outputFeedback.end (); ++message)
          context.error (*message);

        throw 0;  // This is how hooks silently terminate processing.
      }
    }

    // Transfer the modified task back to the original task.
    task = Task (input[0]);
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
// - emitted JSON for the input task is saved, if the exit code is zero,
//   otherwise ignored.
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onModify (const Task& before, Task& after)
{
  if (! _enabled)
    return;

  context.timer_hooks.start ();

  std::vector <std::string> matchingScripts = scripts ("on-modify");
  if (matchingScripts.size ())
  {
    // Convert vector of tasks to a vector of strings.
    std::vector <std::string> input;
    input.push_back (before.composeJSON ()); // [line 0] original, never changes
    input.push_back (after.composeJSON ());  // [line 1] modified

    // Call the hook scripts.
    std::vector <std::string>::iterator script;
    for (script = matchingScripts.begin (); script != matchingScripts.end (); ++script)
    {
      std::vector <std::string> output;
      int status = callHookScript (*script, input, output);

      std::vector <std::string> outputJSON;
      std::vector <std::string> outputFeedback;
      separateOutput (output, outputJSON, outputFeedback);

      if (status == 0)
      {
        assertNTasks    (outputJSON, 1);
        assertValidJSON (outputJSON);
        assertSameTask  (outputJSON, before);

        // Propagate accepted changes forward to the next script.
        input[1] = outputJSON[0];

        std::vector <std::string>::iterator message;
        for (message = outputFeedback.begin (); message != outputFeedback.end (); ++message)
          context.footnote (*message);
      }
      else
      {
        assertFeedback (outputFeedback);

        std::vector <std::string>::iterator message;
        for (message = outputFeedback.begin (); message != outputFeedback.end (); ++message)
          context.error (*message);

        throw 0;  // This is how hooks silently terminate processing.
      }
    }

    after = Task (input[1]);
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
void Hooks::separateOutput (
  const std::vector <std::string>& output,
  std::vector <std::string>& json,
  std::vector <std::string>& feedback) const
{
  std::vector <std::string>::const_iterator i;
  for (i = output.begin (); i != output.end (); ++i)
  {
    if (isJSON (*i))
      json.push_back (*i);
    else
      feedback.push_back (*i);
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::isJSON (const std::string& input) const
{
  return input.length ()            >  2   &&
         input[0]                   == '{' &&
         input[input.length () - 1] == '}';
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::assertValidJSON (const std::vector <std::string>& input) const
{
  std::vector <std::string>::const_iterator i;
  for (i = input.begin (); i != input.end (); i++)
  {
    if (i->length () < 3 ||
        (*i)[0] != '{'   ||
        (*i)[i->length () - 1] != '}')
    {
      context.error (STRING_HOOK_ERROR_OBJECT);
      throw 0;
    }

    try
    {
      json::value* root = json::parse (*i);
      if (root->type () != json::j_object)
      {
        context.error (STRING_HOOK_ERROR_OBJECT);
        throw 0;
      }

      if (((json::object*)root)->_data.find ("description") == ((json::object*)root)->_data.end ())
      {
        context.error (STRING_HOOK_ERROR_NODESC);
        throw 0;
      }

      if (((json::object*)root)->_data.find ("uuid") == ((json::object*)root)->_data.end ())
      {
        context.error (STRING_HOOK_ERROR_NOUUID);
        throw 0;
      }
    }

    catch (const std::string& e)
    {
      context.error (format (STRING_HOOK_ERROR_SYNTAX, *i));
      if (_debug)
        context.error (STRING_HOOK_ERROR_JSON + e);
      throw 0;
    }

    catch (...)
    {
      context.error (STRING_HOOK_ERROR_NOPARSE + *i);
      throw 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::assertNTasks (const std::vector <std::string>& input, int n) const
{
  if (input.size () != n)
  {
    context.error (format (STRING_HOOK_ERROR_BAD_NUM, n, (int) input.size ()));
    throw 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::assertSameTask (const std::vector <std::string>& input, const Task& task) const
{
  std::string uuid = task.get ("uuid");

  std::vector <std::string>::const_iterator i;
  for (i = input.begin (); i != input.end (); i++)
  {
    json::object* root_obj = (json::object*)json::parse (*i);

    // If there is no UUID at all.
    json_object_iter u = root_obj->_data.find ("uuid");
    if (u == root_obj->_data.end ()          ||
        u->second->type () != json::j_string)
    {
      context.error (format (STRING_HOOK_ERROR_SAME1, uuid));
      throw 0;
    }

    json::string* up = (json::string*) u->second;
    std::string json_uuid = json::decode (unquoteText (up->dump ()));
    if (json_uuid != uuid)
    {
      context.error (format (STRING_HOOK_ERROR_SAME2, uuid, json_uuid));
      throw 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::assertFeedback (const std::vector <std::string>& input) const
{
  bool foundSomething = false;
  std::vector <std::string>::const_iterator i;
  for (i = input.begin (); i != input.end (); ++i)
    if (nontrivial (*i))
      foundSomething = true;

  if (! foundSomething)
  {
    context.error (STRING_HOOK_ERROR_NOFEEDBACK);
    throw 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
int Hooks::callHookScript (
  const std::string& script,
  const std::vector <std::string>& input,
  std::vector <std::string>& output)
{
  if (_debug >= 1)
    context.debug ("Hook: Calling " + script);

  if (_debug >= 2)
  {
    context.debug ("Hook: input");
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
  int status;

  // Measure time for each hook if running in debug
  if (_debug >= 2)
  {
    Timer timer_per_hook("Hooks::execute (" + script + ")");
    timer_per_hook.start();

    status = execute (script, args, inputStr, outputStr);
  }
  else
    status = execute (script, args, inputStr, outputStr);

  split (output, outputStr, '\n');

  if (_debug >= 2)
  {
    context.debug ("Hook: output");
    std::vector <std::string>::iterator i;
    for (i = output.begin (); i != output.end (); ++i)
      if (*i != "")
        context.debug ("  " + *i);

    context.debug (format ("Hook: Completed with status {1}", status));
    context.debug (" "); // Blank line
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
