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

#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include <Att.h>
#include <Context.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include "../cmake.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
bool taskDiff (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  foreach (att, before)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  foreach (att, after)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  if (beforeOnly.size () !=
      afterOnly.size ())
    return true;

  foreach (name, beforeAtts)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string taskDifferences (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  foreach (att, before)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  foreach (att, after)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  // Now start generating a description of the differences.
  std::stringstream out;
  foreach (name, beforeOnly)
    out << "  - "
        << ucFirst(*name)
        << " will be deleted.\n";

  foreach (name, afterOnly)
  {
    if (*name == "depends")
    {
      std::vector <int> deps_after;
      after.getDependencies (deps_after);
      std::string to;
      join (to, ", ", deps_after);

      out << "  - "
          << "Dependencies"
          << " will be set to '"
          << to
          << "'.\n";
    }
    else
      out << "  - "
          << ucFirst(*name)
          << " will be set to '"
          << renderAttribute (*name, after.get (*name))
          << "'.\n";
  }

  foreach (name, beforeAtts)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name))
    {
      if (*name == "depends")
      {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        std::vector <int> deps_after;
        after.getDependencies (deps_after);
        std::string to;
        join (to, ", ", deps_after);

        out << "  - "
            << "Dependencies"
            << " will be changed from '"
            << from
            << "' to '"
            << to
            << "'.\n";
      }
      else
        out << "  - "
            << ucFirst(*name)
            << " will be changed from '"
            << renderAttribute (*name, before.get (*name))
            << "' to '"
            << renderAttribute (*name, after.get (*name))
            << "'.\n";
    }

  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << "  - No changes will be made.\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string taskInfoDifferences (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  foreach (att, before)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  foreach (att, after)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  // Now start generating a description of the differences.
  std::stringstream out;
  foreach (name, beforeOnly)
  {
    if (*name == "depends")
    {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        out << "Dependencies '"
            << from
            << "' deleted.\n";
    }
    else if (name->substr (0, 11) == "annotation_")
    {
      out << "Annotation '"
          << before.get (*name)
          << "' deleted.\n";
    }
    else
    {
      out << ucFirst (*name)
          << " deleted.\n";
    }
  }

  foreach (name, afterOnly)
  {
    if (*name == "depends")
    {
      std::vector <int> deps_after;
      after.getDependencies (deps_after);
      std::string to;
      join (to, ", ", deps_after);

      out << "Dependencies"
          << " set to '"
          << to
          << "'.\n";
    }
    else if (name->substr (0, 11) == "annotation_")
    {
      out << "Annotation of '"
          << after.get (*name)
          << "' added.\n";
    }
    else
      out << ucFirst(*name)
          << " set to '"
          << renderAttribute (*name, after.get (*name))
          << "'.\n";
  }

  foreach (name, beforeAtts)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name) &&
        before.get (*name) != "" && after.get (*name) != "")
    {
      if (*name == "depends")
      {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        std::vector <int> deps_after;
        after.getDependencies (deps_after);
        std::string to;
        join (to, ", ", deps_after);

        out << "Dependencies"
            << " changed from '"
            << from
            << "' to '"
            << to
            << "'.\n";
      }
      else if (name->substr (0, 11) == "annotation_")
      {
        out << "Annotation changed to '"
            << after.get (*name)
            << "'.\n";
      }
      else
        out << ucFirst(*name)
            << " changed from '"
            << renderAttribute (*name, before.get (*name))
            << "' to '"
            << renderAttribute (*name, after.get (*name))
            << "'.\n";
    }

  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << "No changes made.\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string renderAttribute (const std::string& name, const std::string& value)
{
  Att a;
  if (a.type (name) == "date" &&
      value != "")
  {
    Date d ((time_t)strtol (value.c_str (), NULL, 10));
    return d.toString (context.config.get ("dateformat"));
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Implement all the post-command feedback here.  This includes project
//      completion percentages, "3 tasks modified", all warnings, and so on.
std::string feedback (const Task& before, const Task& after)
{
}

////////////////////////////////////////////////////////////////////////////////

