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

#include <iostream> // TODO Remove
#include <cmake.h>
#include <algorithm>
#include <stdio.h>
#include <Context.h>
#include <Hooks.h>
#include <text.h>

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
// Occurs when:       On launch, after data structures are initiliazed, before
//                    data is loaded.
// Data fed to stdin: None
// Exit code:         0: Success, proceed
//                    !0: Failure, terminate
// Output handled:    0:  context.header ()
//                    !0: context.error ()
void Hooks::onLaunch ()
{
  context.timer_hooks.start ();

  std::vector <std::string>::iterator i;
  for (i = _scripts.begin (); i != _scripts.end (); ++i)
  {
    if (i->find ("/on-launch") != std::string::npos)
    {
      File script (*i);
      if (script.executable ())
      {
        std::string output;
        int status = execute (*i, "", output);

        std::vector <std::string> lines;
        split (lines, output, '\n');
        std::vector <std::string>::iterator line;

        if (status == 0)
        {
          for (line = lines.begin (); line != lines.end (); ++line)
            context.header (*line);
        }
        else
        {
          for (line = lines.begin (); line != lines.end (); ++line)
            context.error (*line);

          throw 0;  // This is how hooks silently terminate processing.
        }
      }
    }
  }

  context.timer_hooks.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// Occurs when:       On exit, after processing is complete, before output is
//                    displayed.
// Data fed to stdin: None
// Exit code:         0: Success
//                    !0: Failure
// Output handled:    0:  context.footnote ()
//                    !0: context.error ()
void Hooks::onExit ()
{
  context.timer_hooks.start ();

  std::vector <std::string>::iterator i;
  for (i = _scripts.begin (); i != _scripts.end (); ++i)
  {
    if (i->substr (0, 7) == "on-exit")
    {
      File script (*i);
      if (script.executable ())
      {
        // TODO Call all exit hook scripts.

        // TODO On zero status:
        //      - all stdout --> context.footnote

        // TODO On non-zero status:
        //      - all stdout --> context.error
      }
    }
  }

  context.timer_hooks.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// Occurs when:       A task is created, before it is committed.
// Data fed to stdin: task JSON
// Exit code:         0: Success
//                    !0: Failure
// Output handled:    0:  modified JSON
//                        context.footnote ()
//                    !0: context.error ()
void Hooks::onAdd (Task& after)
{
  context.timer_hooks.start ();

  std::vector <std::string>::iterator i;
  for (i = _scripts.begin (); i != _scripts.end (); ++i)
  {
    if (i->substr (0, 6) == "on-add")
    {
      File script (*i);
      if (script.executable ())
      {
        // TODO Call all modify hook scripts.

        // TODO On zero status:
        //      - first line is modified JSON
        //      - remaining lines --> context.footnote

        // TODO On non-zero status:
        //      - all stdout --> context.error
      }
    }
  }

  context.timer_hooks.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// Occurs when:       A task is modified, before it is committed.
// Data fed to stdin: before JSON
//                    after JSON
// Exit code:         0: Success
//                    !0: Failure
// Output handled:    0:  modified after JSON
//                        context.footnote ()
//                    !0: context.error ()
void Hooks::onModify (const Task& before, Task& after)
{
  context.timer_hooks.start ();

  std::vector <std::string>::iterator i;
  for (i = _scripts.begin (); i != _scripts.end (); ++i)
  {
    if (i->substr (0, 9) == "on-modify")
    {
      File script (*i);
      if (script.executable ())
      {
        // TODO Call all modify hook scripts.

        // TODO On zero status:
        //      - first line is modified JSON
        //      - remaining lines --> context.footnote

        // TODO On non-zero status:
        //      - all stdout --> context.error
      }
    }
  }

  context.timer_hooks.stop ();
}

////////////////////////////////////////////////////////////////////////////////
int Hooks::execute (
  const std::string& command,
  const std::string& input,
  std::string& output)
{
  FILE* fp = popen (command.c_str (), "r+");
  if (fp)
  {
    // Write input to fp.
    if (input != "")
    {
      fputs (input.c_str (), fp);
      fflush (fp);
    }

    // Read output from fp.
    output = "";
    char* line = NULL;
    size_t len = 0;
    while (getline (&line, &len, fp) != -1)
    {
      output += line;
      free (line);
      line = NULL;
    }

    fflush (fp);
    return pclose (fp);
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
