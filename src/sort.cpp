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
#include <vector>
#include <string>
#include <stdlib.h>
#include <Context.h>
#include <Variant.h>
#include <Duration.h>
#include <Task.h>
#include <text.h>
#include <i18n.h>

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
  context.timer_sort.start ();

  global_data = &data;

  // Split the key defs.
  global_keys.clear ();
  split (global_keys, keys, ',');

  // Only sort if necessary.
  if (order.size ())
    std::stable_sort (order.begin (), order.end (), sort_compare);

  context.timer_sort.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// Re-implementation, using direct Task access instead of data copies that
// require re-parsing.
//
// Essentially a static implementation of a dynamic operator<.
// UDA string values delegate to Variant::operator<.
static bool sort_compare (int left, int right)
{
  std::string field;
  bool ascending;
  bool breakIndicator;
  Column* column;
  int left_number;
  int right_number;
  float left_real;
  float right_real;

  for (auto& k : global_keys)
  {
    context.decomposeSortField (k, field, ascending, breakIndicator);

    // Urgency.
    if (field == "urgency")
    {
      left_real  = (*global_data)[left].urgency ();
      right_real = (*global_data)[right].urgency ();

      if (left_real == right_real)
        continue;

      return ascending ? (left_real < right_real)
                       : (left_real > right_real);
    }

    // Number.
    else if (field == "id")
    {
      left_number  = (*global_data)[left].id;
      right_number = (*global_data)[right].id;

      if (left_number == right_number)
        continue;

      return ascending ? (left_number < right_number)
                       : (left_number > right_number);
    }

    // String.
    else if (field == "description" ||
             field == "project"     ||
             field == "status"      ||
             field == "tags"        ||
             field == "uuid"        ||
             field == "parent"      ||
             field == "imask"       ||
             field == "mask")
    {
      const std::string& left_string  = (*global_data)[left].get_ref  (field);
      const std::string& right_string = (*global_data)[right].get_ref (field);

      if (left_string == right_string)
        continue;

      return ascending ? (left_string < right_string)
                       : (left_string > right_string);
    }

    // Due Date.
    else if (field == "due")
    {
      const std::string& left_string  = (*global_data)[left].get_ref  (field);
      const std::string& right_string = (*global_data)[right].get_ref (field);

      if (left_string != "" && right_string == "")
        return true;

      if (left_string == "" && right_string != "")
        return false;

      if (left_string == right_string)
        continue;

      return ascending ? (left_string < right_string)
                       : (left_string > right_string);
    }

    // Date.
    else if (field == "end"      ||
             field == "entry"    ||
             field == "start"    ||
             field == "until"    ||
             field == "wait"     ||
             field == "modified" ||
             field == "scheduled")
    {
      const std::string& left_string  = (*global_data)[left].get_ref  (field);
      const std::string& right_string = (*global_data)[right].get_ref (field);

      if (left_string == right_string)
        continue;

      return ascending ? (left_string < right_string)
                       : (left_string > right_string);
    }

    // Depends string.
    else if (field == "depends")
    {
      // Raw data is a comma-separated list of uuids
      const std::string& left_string  = (*global_data)[left].get_ref  (field);
      const std::string& right_string = (*global_data)[right].get_ref (field);

      if (left_string == right_string)
        continue;

      if (left_string == "" && right_string != "")
        return ascending;

      if (left_string != "" && right_string == "")
        return !ascending;

      // Sort on the first dependency.
      left_number  = context.tdb2.id (left_string.substr (0, 36));
      right_number = context.tdb2.id (right_string.substr (0, 36));

      if (left_number == right_number)
        continue;

      return ascending ? (left_number < right_number)
                       : (left_number > right_number);
    }

    // Duration.
    else if (field == "recur")
    {
      const std::string& left_string  = (*global_data)[left].get_ref  (field);
      const std::string& right_string = (*global_data)[right].get_ref (field);

      if (left_string == right_string)
        continue;

      Duration left_duration (left_string);
      Duration right_duration (right_string);
      return ascending ? (left_duration < right_duration)
                       : (left_duration > right_duration);
    }

    // UDAs.
    else if ((column = context.columns[field]) != NULL)
    {
      std::string type = column->type ();
      if (type == "numeric")
      {
        const float left_real  = strtof (((*global_data)[left].get_ref  (field)).c_str (), NULL);
        const float right_real = strtof (((*global_data)[right].get_ref (field)).c_str (), NULL);

        if (left_real == right_real)
          continue;

        return ascending ? (left_real < right_real)
                         : (left_real > right_real);
      }
      // UDA values of type 'string' are sorted by Variant::operator<.
      // By setting 'source' to the UDA name, the comparison operator can use
      // the custom sort order, if defined.
      else if (type == "string")
      {
        Variant l ((*global_data)[left].get_ref  (field));
        Variant r ((*global_data)[right].get_ref (field));
        if (l == r)
          continue;

        l.source (field);
        r.source (field);
        return ascending ? (l < r) : (r < l);
      }
      else if (type == "date")
      {
        const std::string& left_string  = (*global_data)[left].get_ref  (field);
        const std::string& right_string = (*global_data)[right].get_ref (field);

        if (left_string == right_string)
          continue;

        return ascending ? (left_string < right_string)
                         : (left_string > right_string);
      }
      else if (type == "duration")
      {
        const std::string& left_string  = (*global_data)[left].get_ref  (field);
        const std::string& right_string = (*global_data)[right].get_ref (field);

        if (left_string == right_string)
          continue;

        Duration left_duration (left_string);
        Duration right_duration (right_string);
        return ascending ? (left_duration < right_duration)
                         : (left_duration > right_duration);
      }
    }
    else
      throw format (STRING_INVALID_SORT_COL, field);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
