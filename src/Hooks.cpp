////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

#include <iostream>
#include <algorithm>
#include <Context.h>
#include <Hooks.h>
#include <Timer.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Hook::Hook ()
: _event ("")
, _file ("")
, _function ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Hook::Hook (const std::string& e, const std::string& f, const std::string& fn)
: _event (e)
, _file (f)
, _function (fn)
{
}

////////////////////////////////////////////////////////////////////////////////
Hook::Hook (const Hook& other)
{
  _event = other._event;
  _file = other._file;
  _function = other._function;
}

////////////////////////////////////////////////////////////////////////////////
Hook& Hook::operator= (const Hook& other)
{
  if (this != &other)
  {
    _event = other._event;
    _file = other._file;
    _function = other._function;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Hooks::Hooks ()
{
  // New 2.x hooks.
  _validTaskEvents.push_back ("on-task-add");       // Unimplemented
  _validTaskEvents.push_back ("on-task-modify");    // Unimplemented
  _validTaskEvents.push_back ("on-task-complete");  // Unimplemented
  _validTaskEvents.push_back ("on-task-delete");    // Unimplemented

  _validProgramEvents.push_back ("on-launch");
  _validProgramEvents.push_back ("on-exit");
  _validProgramEvents.push_back ("on-file-read");   // Unimplemented
  _validProgramEvents.push_back ("on-file-write");  // Unimplemented
  _validProgramEvents.push_back ("on-synch");       // Unimplemented
  _validProgramEvents.push_back ("on-merge");       // Unimplemented
  _validProgramEvents.push_back ("on-gc");          // Unimplemented
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
  // Allow a master switch to turn the whole thing off.
  bool big_red_switch = context.config.getBoolean ("extensions");
  if (big_red_switch)
  {
    Config::const_iterator it;
    for (it = context.config.begin (); it != context.config.end (); ++it)
    {
      std::string type;
      std::string name;
      std::string value;

      // "<type>.<name>"
      Nibbler n (it->first);
      if (n.getUntil ('.', type) &&
          type == "hook"         &&
          n.skip ('.')           &&
          n.getUntilEOS (name))
      {
        Nibbler n (it->second);

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
            _all.push_back (h);

            (void) n.skip (',');
          }
          else
            ; // Was: throw std::string (format ("Malformed hook definition '{1}'.", it->first));
        }
      }
    }
  }
  else
    context.debug ("Hooks::initialize --> off");
}

////////////////////////////////////////////////////////////////////////////////
// Program hooks.
bool Hooks::trigger (const std::string& event)
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Task hooks.
bool Hooks::trigger (const std::string& event, Task& task)
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::validProgramEvent (const std::string& event)
{
  if (std::find (_validProgramEvents.begin (), _validProgramEvents.end (), event) != _validProgramEvents.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::validTaskEvent (const std::string& event)
{
  if (std::find (_validTaskEvents.begin (), _validTaskEvents.end (), event) != _validTaskEvents.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
