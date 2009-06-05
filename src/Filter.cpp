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

#include "Filter.h"
#include "util.h"

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
