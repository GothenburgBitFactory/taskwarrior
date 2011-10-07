////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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


#define L10N                                           // Localization complete.

#include <iostream>
#include <Permission.h>
#include <Context.h>
#include <util.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Permission::Permission ()
: _need_confirmation (false)
, _all_confirmed (false)
, _quit (false)
{
  // Turning confirmations off is the same as entering "all".
  if (context.config.getBoolean ("confirmation") == false)
    _all_confirmed = true;
}

////////////////////////////////////////////////////////////////////////////////
bool Permission::confirmed (const Task& task, const std::string& question)
{
  if (_quit)
    return false;

  if (!_need_confirmation)
    return true;

  if (_all_confirmed)
    return true;

  std::cout << "\n"
            << format (STRING_PERM_TASK_LINE, task.id, task.get ("description"));

  if (task.getStatus () == Task::recurring ||
      task.has ("parent"))
  {
    std::cout << " "
              << STRING_PERM_RECURRING;
  }

  std::cout << std::endl;  // Flush.

  int answer = confirm4 (question);
  std::cout << "\n";       // #499

  if (answer == 2)
    _all_confirmed = true;

  if (answer == 1 || answer == 2)
    return true;

  if (answer == 3)
    _quit = true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
