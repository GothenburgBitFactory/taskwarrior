////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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

#include <sstream>
#include "Filter.h"
#include "util.h"
#include "main.h"

////////////////////////////////////////////////////////////////////////////////
// For every Att in the filter, lookup the equivalent in Record, and perform a
// match.  Aren't filters easy now that everything is an attribute?
bool Filter::pass (const Record& record) const
{
  Record::const_iterator r;

  // First do description/annotation matches.
  foreach (att, (*this))
  {
    // Descriptions have special handling such that they are linked to
    // annotations, and filtering on description implies identical filtering
    // on annotations.
    //
    // For positive modifiers (has, is ...) either the description or the
    // annotation filter must succeed.
    //
    // For negative modifiers (hasnt, isnt ...) both the description and the
    // annotation filter must succeed.
    if (att->name () == "description")
    {
      bool pass = true;
      int annotation_pass_count = 0;
      int annotation_fail_count = 0;

      if ((r = record.find (att->name ())) != record.end ())
      {
        pass = att->match (r->second);

        foreach (ra, record)
        {
          if (ra->first.length () > 11 &&
              ra->first.substr (0, 11) == "annotation_")
          {
            if (att->match (ra->second))
              ++annotation_pass_count;
            else
              ++annotation_fail_count;
          }
        }
      }

      // Missing attribute implies failure.
      else if (! att->match (Att ()))
        return false;

      // This innocuous little if-else took significantly more thinking and
      // debugging than anything else in task.  Only change this code if you
      // are willing to invest a week understanding and testing it.
      if (att->modType (att->mod ()) == "positive")
      {
        if (! att->logicSense (pass || annotation_pass_count))
          return false;
      }
      else
      {
        if (! att->logicSense (pass && annotation_fail_count == 0))
          return false;
      }
    }

    // Annotations are skipped, because they are handled above.
    else if (att->name ().length () > 11 &&
             att->name ().substr (0, 11) == "annotation_")
    {
    }

    else
    {
      // An individual attribute match failure is enough to fail the filter.
      if ((r = record.find (att->name ())) != record.end ())
      {
        if (! att->logicSense (att->match (r->second)))
          return false;
      }
      else if (! att->match (Att ()))
        return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
void Filter::applySequence (std::vector<Task>& all, Sequence& sequence)
{
  std::vector <Task> filtered;
  foreach (task, all)
    foreach (i, sequence)
      if (task->id == *i)
        filtered.push_back (*task);

  if (sequence.size () != filtered.size ())
  {
    std::vector <int> filteredSequence;
    foreach (task, filtered)
      filteredSequence.push_back (task->id);

    std::vector <int> left;
    std::vector <int> right;
    listDiff (filteredSequence, (std::vector <int>&)sequence, left, right);
    if (left.size ())
      throw std::string ("Sequence filtering error - please report this error to support@taskwarrior.org.");

    if (right.size ())
    {
      std::stringstream out;
      out << "Task";

      if (right.size () > 1) out << "s";

      foreach (r, right)
        out << " " << *r;

      out << " not found.";
      throw out.str ();
    }
  }

  all = filtered;
}

////////////////////////////////////////////////////////////////////////////////
