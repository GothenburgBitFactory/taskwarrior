////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2019, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <Context.h>

////////////////////////////////////////////////////////////////////////////////
// Returns a Boolean indicator as to whether a nag message was generated, so
// that commands can control the number of nag messages displayed (ie one is
// enough).
//
// Otherwise generates a nag message, if one is defined, if there are tasks of
// higher urgency.
bool nag (Task& task)
{
  // Special tag overrides nagging.
  if (task.hasTag ("nonag"))
    return false;

  auto msg = Context::getContext ().config.get ("nag");
  if (msg != "")
  {
    // Scan all pending, non-recurring tasks.
    auto pending = Context::getContext ().tdb2.pending.get_tasks ();
    for (auto& t : pending)
    {
      if ((t.getStatus () == Task::pending  ||
           t.getStatus () == Task::waiting) &&
          t.hasTag ("READY")                &&
          t.urgency () > task.urgency ())
      {
        Context::getContext ().footnote (msg);
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
