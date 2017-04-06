////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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
#include <Datetime.h>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Checklist:
// - last: Most recently generated instance integer. The first instance
//   generated is '1'.
// - Sync must merge duplicate N.
// - Remove recurrence.limit. Now always 1. It is not something that can be done
//   with rtype:chained tasks.
// - Handle until.

////////////////////////////////////////////////////////////////////////////////
// Given an old-style task, upgrades it perfectly.
// Note: Works for both parent and child.
static Task upgradeTask (const Task&)
{
  Task upgraded;

  return upgraded;
}

////////////////////////////////////////////////////////////////////////////////
// Calculates the due date for a new new instance N.
static Datetime calculateDueN (const Task&, int)
{
  Datetime due;

  return due;
}

////////////////////////////////////////////////////////////////////////////////
static void synthesizeTasks (const Task&)
{
  context.debug ("synthesizeTasks start");

  // TODO 'due' = starting point
  // TODO 'recur' = frequency
  // TODO 'last' = index of most recently synthesized instance

  context.debug ("synthesizeTasks end");
}

////////////////////////////////////////////////////////////////////////////////
// Generates all necessary recurring task instances.
void handleRecurrence2 ()
{
  // Note: Disabling recurrence is currently a workaround for TD-44, TW-1520.
  if (context.config.getBoolean ("recurrence"))
    for (auto& t : context.tdb2.pending.get_tasks ())
      if (t.getStatus () == Task::recurring)
        synthesizeTasks (t);
}

////////////////////////////////////////////////////////////////////////////////
