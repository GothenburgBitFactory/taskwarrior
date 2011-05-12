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

#include <algorithm>
#include <vector>
#include <string>
#include <Context.h>
#include <Duration.h>
#include <Task.h>
#include <Timer.h>
#include <text.h>

extern Context context;

static std::vector <Task>* global_data = NULL;
static std::vector <std::string> global_keys;
static bool sort_compare (int, int);

////////////////////////////////////////////////////////////////////////////////
void sort_tasks (
  std::vector <Task>& data,
  std::vector <int>& order,
  const std::string& keys)
{
  Timer t ("Sort");

  global_data = &data;

  // Split the key defs.
  global_keys.clear ();
  split (global_keys, keys, ',');

  // Only sort if necessary.
  if (order.size ())
    std::stable_sort (order.begin (), order.end (), sort_compare);
}

////////////////////////////////////////////////////////////////////////////////
// Re-implementation, using direct Task access instead of data copies that
// require re-parsing.
//
// Essentially a static implementation of a dynamic operator<.
static bool sort_compare (int left, int right)
{
  int result;
  std::string field;
  bool ascending;

  int left_number;
  int right_number;
  std::string left_string;
  std::string right_string;
  time_t left_date;
  time_t right_date;
  float left_real;
  float right_real;

  std::vector <std::string>::iterator k;
  for (k = global_keys.begin (); k != global_keys.end (); ++k)
  {
    context.decomposeSortField (*k, field, ascending);

    // Number.
    if (field == "id")
    {
      left_number  = (*global_data)[left].id;
      right_number = (*global_data)[right].id;

      if (left_number == right_number)
        continue;

      if (ascending)
        return left_number < right_number;

      return left_number > right_number;
    }

    // String.
    else if (field == "description" ||
             field == "depends"     ||
             field == "project"     ||
             field == "status"      ||
             field == "tags"        ||
             field == "uuid")
    {
      left_string  = (*global_data)[left].get  (field);
      right_string = (*global_data)[right].get (field);

      if (left_string == right_string)
        continue;

      if (ascending)
        return left_string < right_string;

      return left_string > right_string;
    }

    // Priority.
    else if (field == "priority")
    {
      left_string  = (*global_data)[left].get  (field);
      right_string = (*global_data)[right].get (field);
      if (left_string == right_string)
        continue;

      if (ascending)
        return (left_string == ""  && right_string != "")                           ||
               (left_string == "L" && (right_string == "M" || right_string == "H")) ||
               (left_string == "M" && right_string == "H");

      return (left_string != ""  && right_string == "")                           ||
             (left_string == "M" && right_string == "L")                          ||
             (left_string == "H" && (right_string == "M" || right_string == "L"));
    }

    // Due Date.
    else if (field == "due")
    {
      left_string  = (*global_data)[left].get  (field);
      right_string = (*global_data)[right].get (field);

      if (left_string != "" && right_string == "")
        return true;

      if (left_string == "" && right_string != "")
        return false;

      if (left_string == right_string)
        continue;

      left_date  = atoi (left_string.c_str ());
      right_date = atoi (right_string.c_str ());

      if (ascending)
        return left_date < right_date;

      return left_date > right_date;
    }

    // Date.
    else if (field == "end"   ||
             field == "entry" ||
             field == "start" ||
             field == "until" ||
             field == "wait")
    {
      left_string  = (*global_data)[left].get  (field);
      right_string = (*global_data)[right].get (field);

      if (left_string == right_string)
        continue;

      left_date  = atoi (left_string.c_str ());
      right_date = atoi (right_string.c_str ());

      if (ascending)
        return left_date < right_date;

      return left_date > right_date;
    }

    // Duration.
    else if (field == "recur")
    {
      left_string  = (*global_data)[left].get  (field);
      right_string = (*global_data)[right].get (field);

      if (left_string == right_string)
        continue;

      Duration left_duration (left_string);
      Duration right_duration (right_string);
      if (ascending)
        return left_duration < right_duration;

      return left_duration > right_duration;
    }

    // Urgency.
    else if (field == "urgency")
    {
      left_real  = (*global_data)[left].urgency ();
      right_real = (*global_data)[right].urgency ();

      if (left_real == right_real)
        continue;

      if (ascending)
        return left_real < right_real;

      return left_real > right_real;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
