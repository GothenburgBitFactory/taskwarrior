////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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

  // If record doesn't have the attribute, fail.  If it does have the attribute
  // but it doesn't match, fail.
  foreach (att, (*this))
  {
    // If the record doesn't have the attribute, match against a default one.
    // This is because "att" may contain a modifier like "name.not:X".
    if ((r = record.find (att->name ())) == record.end ())
    {
      if (! att->match (Att ()))
        return false;
    }
    else if (! att->match (r->second))
      return false;
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
      throw std::string ("Sequence filtering error - please report this error");

    if (right.size ())
    {
      std::stringstream out;
      out << "Task";

      if (right.size () > 1) out << "s";

      foreach (r, right)
        out << " " << *r;

      out << " not found";
      throw out.str ();
    }
  }

  all = filtered;
}

////////////////////////////////////////////////////////////////////////////////
