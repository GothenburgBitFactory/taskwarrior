////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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

//#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <Context.h>
#include <Nibbler.h>
#include <Date.h>
#include <Duration.h>
#include <Task.h>
#include <JSON.h>
#include <RX.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>

#define APPROACHING_INFINITY 1000   // Close enough.  This isn't rocket surgery.

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Task::Task ()
: id (0)
, urgency_value (0.0)
, recalc_urgency (true)
{
}

////////////////////////////////////////////////////////////////////////////////
Task::Task (const Task& other)
{
  *this = other;
}

////////////////////////////////////////////////////////////////////////////////
Task& Task::operator= (const Task& other)
{
  if (this != &other)
  {
    std::map <std::string, std::string>::operator= (other);

    id             = other.id;
    urgency_value  = other.urgency_value;
    recalc_urgency = other.recalc_urgency;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// The uuid and id attributes must be exempt from comparison.
bool Task::operator== (const Task& other)
{
  if (size () != other.size ())
    return false;

  Task::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    if (i->first != "uuid" &&
        i->second != other.get (i->first))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
Task::Task (const std::string& input)
{
  id = 0;
  urgency_value = 0.0;
  recalc_urgency = true;
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Task::~Task ()
{
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::textToStatus (const std::string& input)
{
       if (input[0] == 'p') return Task::pending;
  else if (input[0] == 'c') return Task::completed;
  else if (input[0] == 'd') return Task::deleted;
  else if (input[0] == 'r') return Task::recurring;
  else if (input[0] == 'w') return Task::waiting;

  return Task::pending;
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::statusToText (Task::status s)
{
       if (s == Task::pending)   return "pending";
  else if (s == Task::completed) return "completed";
  else if (s == Task::deleted)   return "deleted";
  else if (s == Task::recurring) return "recurring";
  else if (s == Task::waiting)   return "waiting";

  return "pending";
}

////////////////////////////////////////////////////////////////////////////////
void Task::setEntry ()
{
  char entryTime[16];
  sprintf (entryTime, "%u", (unsigned int) time (NULL));
  set ("entry", entryTime);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::setEnd ()
{
  char endTime[16];
  sprintf (endTime, "%u", (unsigned int) time (NULL));
  set ("end", endTime);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::setStart ()
{
  char startTime[16];
  sprintf (startTime, "%u", (unsigned int) time (NULL));
  set ("start", startTime);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::has (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Task::all ()
{
  std::vector <std::string> all;
  Task::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    all.push_back (i->first);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Task::get (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int Task::get_int (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return strtol (i->second.c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
unsigned long Task::get_ulong (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return strtoul (i->second.c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
time_t Task::get_date (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return (time_t) strtoul (i->second.c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
time_t Task::get_duration (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return (time_t) strtoul (i->second.c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Task::set (const std::string& name, const std::string& value)
{
  (*this)[name] = value;

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::set (const std::string& name, int value)
{
  (*this)[name] = format (value);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::remove (const std::string& name)
{
  Task::iterator it;
  if ((it = this->find (name)) != this->end ())
    this->erase (it);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::getStatus () const
{
  return textToStatus (get ("status"));
}

////////////////////////////////////////////////////////////////////////////////
void Task::setStatus (Task::status status)
{
  set ("status", statusToText (status));

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
// Attempt an FF4 parse first, using Task::parse, and in the event of an error
// try a legacy parse (F3, FF2).  Note that FF1 is no longer supported.
//
// start --> [ --> Att --> ] --> end
//              ^       |
//              +-------+
//
void Task::parse (const std::string& input)
{
  std::string copy;
  if (input[input.length () - 1] == '\n')
    copy = input.substr (0, input.length () - 1);
  else
    copy = input;

  try
  {
    clear ();

    Nibbler n (copy);
    std::string line;
    if (n.skip     ('[')       &&
        n.getUntil (']', line) &&
        n.skip     (']')       &&
        n.depleted ())
    {
      if (line.length () == 0)
        throw std::string (STRING_RECORD_EMPTY);

      Nibbler nl (line);
      std::string name;
      std::string value;
      while (!nl.depleted ())
      {
        if (nl.getUntil (':', name) &&
            nl.skip (':')           &&
            nl.getQuoted ('"', value))
        {
          // Experimental legacy value translation of 'recur:m' --> 'recur:mo'.
          if (name == "recur" &&
              digitsOnly (value.substr (0, value.length () - 1)) &&
              value[value.length () - 1] == 'm')
            value += 'o';

          (*this)[name] = decode (json::decode (value));
        }

        nl.skip (' ');
      }

      std::string remainder;
      nl.getUntilEOS (remainder);
      if (remainder.length ())
        throw std::string (STRING_RECORD_JUNK_AT_EOL);
    }
    else
      throw std::string (STRING_RECORD_NOT_FF4);
  }

  catch (std::string& e)
  {
    legacyParse (copy);
  }

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
// Support FF2, FF3.
// Thankfully FF1 is no longer supported.
void Task::legacyParse (const std::string& line)
{
  switch (determineVersion (line))
  {
  // File format version 1, from 2006.11.27 - 2007.12.31
  case 1:
    throw std::string (STRING_TASK_NO_FF1);
    break;

  // File format version 2, from 2008.1.1 - 2009.3.23
  case 2:
    {
      if (line.length () > 46)       // ^.{36} . \[\] \[\] \n
      {
        set ("uuid", line.substr (0, 36));

        Task::status status = line[37] == '+' ? completed
                            : line[37] == 'X' ? deleted
                            : line[37] == 'r' ? recurring
                            :                   pending;

        set ("status", statusToText (status));

        size_t openTagBracket  = line.find ("[");
        size_t closeTagBracket = line.find ("]", openTagBracket);
        if (openTagBracket  != std::string::npos &&
            closeTagBracket != std::string::npos)
        {
          size_t openAttrBracket  = line.find ("[", closeTagBracket);
          size_t closeAttrBracket = line.find ("]", openAttrBracket);
          if (openAttrBracket  != std::string::npos &&
              closeAttrBracket != std::string::npos)
          {
            std::string tags = line.substr (
              openTagBracket + 1, closeTagBracket - openTagBracket - 1);
            std::vector <std::string> tagSet;
            split (tagSet, tags, ' ');
            addTags (tagSet);

            std::string attributes = line.substr (
              openAttrBracket + 1, closeAttrBracket - openAttrBracket - 1);
            std::vector <std::string> pairs;
            split (pairs, attributes, ' ');
            for (size_t i = 0; i <  pairs.size (); ++i)
            {
              std::vector <std::string> pair;
              split (pair, pairs[i], ':');
              if (pair.size () == 2)
                set (pair[0], pair[1]);
            }

            set ("description", line.substr (closeAttrBracket + 2));
          }
          else
            throw std::string (STRING_TASK_PARSE_ATT_BRACK);
        }
        else
          throw std::string (STRING_TASK_PARSE_TAG_BRACK);
      }
      else
        throw std::string (STRING_TASK_PARSE_TOO_SHORT);

      removeAnnotations ();
    }
    break;

  // File format version 3, from 2009.3.23
  case 3:
    {
      if (line.length () > 49)       // ^.{36} . \[\] \[\] \[\] \n
      {
        set ("uuid", line.substr (0, 36));

        Task::status status = line[37] == '+' ? completed
                            : line[37] == 'X' ? deleted
                            : line[37] == 'r' ? recurring
                            :                   pending;

        set ("status", statusToText (status));

        size_t openTagBracket  = line.find ("[");
        size_t closeTagBracket = line.find ("]", openTagBracket);
        if (openTagBracket  != std::string::npos &&
            closeTagBracket != std::string::npos)
        {
          size_t openAttrBracket  = line.find ("[", closeTagBracket);
          size_t closeAttrBracket = line.find ("]", openAttrBracket);
          if (openAttrBracket  != std::string::npos &&
              closeAttrBracket != std::string::npos)
          {
            size_t openAnnoBracket  = line.find ("[", closeAttrBracket);
            size_t closeAnnoBracket = line.find ("]", openAnnoBracket);
            if (openAnnoBracket  != std::string::npos &&
                closeAnnoBracket != std::string::npos)
            {
              std::string tags = line.substr (
                openTagBracket + 1, closeTagBracket - openTagBracket - 1);
              std::vector <std::string> tagSet;
              split (tagSet, tags, ' ');
              addTags (tagSet);

              std::string attributes = line.substr (
                openAttrBracket + 1, closeAttrBracket - openAttrBracket - 1);
              std::vector <std::string> pairs;
              split (pairs, attributes, ' ');

              for (size_t i = 0; i <  pairs.size (); ++i)
              {
                std::vector <std::string> pair;
                split (pair, pairs[i], ':');
                if (pair.size () == 2)
                  set (pair[0], pair[1]);
              }

              // Extract and split the annotations, which are of the form:
              //   1234:"..." 5678:"..."
              std::string annotations = line.substr (
                openAnnoBracket + 1, closeAnnoBracket - openAnnoBracket - 1);
              pairs.clear ();

              std::string::size_type start = 0;
              std::string::size_type end   = 0;
              do
              {
                end = annotations.find ('"', start);
                if (end != std::string::npos)
                {
                  end = annotations.find ('"', end + 1);

                  if (start != std::string::npos &&
                      end   != std::string::npos)
                  {
                    pairs.push_back (annotations.substr (start, end - start + 1));
                    start = end + 2;
                  }
                }
              }
              while (start != std::string::npos &&
                     end   != std::string::npos);

              for (size_t i = 0; i < pairs.size (); ++i)
              {
                std::string pair = pairs[i];
                std::string::size_type colon = pair.find (":");
                if (colon != std::string::npos)
                {
                  std::string name = pair.substr (0, colon);
                  std::string value = pair.substr (colon + 2, pair.length () - colon - 3);
                  set ("annotation_" + name, value);
                }
              }

              set ("description", line.substr (closeAnnoBracket + 2));
            }
            else
              throw std::string (STRING_TASK_PARSE_ANNO_BRACK);
          }
          else
            throw std::string (STRING_TASK_PARSE_ATT_BRACK);
        }
        else
          throw std::string (STRING_TASK_PARSE_TAG_BRACK);
      }
      else
        throw std::string (STRING_TASK_PARSE_TOO_SHORT);
    }
    break;

  default:
    throw std::string (STRING_TASK_PARSE_UNREC_FF);
    break;
  }

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
// The format is:
//
//   [ <name>:<value> ... ] \n
//
std::string Task::composeF4 () const
{
  std::string ff4 = "[";

  bool first = true;
  Task::const_iterator it;
  for (it = this->begin (); it != this->end (); ++it)
  {
    if (it->second != "")
    {
      ff4 += (first ? "" : " ")
           + it->first
           + ":\"" + encode (json::encode (it->second)) + "\"";
      first = false;
    }
  }

  ff4 += "]\n";
  return ff4;
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::composeJSON (bool include_id /*= false*/) const
{
  std::stringstream out;
  out << "{";

  // ID inclusion is optional, not recommended.
  if (include_id)
    out << "\"id\":" << id << ",";

  // First the non-annotations.
  int attributes_written = 0;
  int annotation_count = 0;
  Task::const_iterator i;
  for (i = this->begin (); i != this->end (); ++i)
  {
    if (attributes_written)
      out << ",";

    Column* column = context.columns[i->first];

    // Annotations are simply counted.
    if (i->first.substr (0, 11) == "annotation_")
    {
      ++annotation_count;
    }

    // Date fields are written as ISO 8601.
    else if (column &&
             column->type () == "date")
    {
      Date d (i->second);
      out << "\""
          << i->first
          << "\":\""
          << d.toISO ()
          << "\"";

      ++attributes_written;
    }

    // Tags are converted to an array.
    else if (i->first == "tags")
    {
      std::vector <std::string> tags;
      split (tags, i->second, ',');

      out << "\"tags\":[";

      std::vector <std::string>::iterator i;
      for (i = tags.begin (); i != tags.end (); ++i)
      {
        if (i != tags.begin ())
          out << ",";

        out << "\"" << *i << "\"";
      }

      out << "]";
    }

    // Everything else is a quoted value.
    else
    {
      out << "\""
          << i->first
          << "\":\""
          << json::encode (i->second)
          << "\"";

      ++attributes_written;
    }
  }

  // Now the annotations, if any.
  if (annotation_count)
  {
    out << ","
        << "\"annotations\":[";

    int annotations_written = 0;
    for (i = this->begin (); i != this->end (); ++i)
    {
      if (i->first.substr (0, 11) == "annotation_")
      {
        if (annotations_written)
          out << ",";

        Date d (i->first.substr (11));
        out << "{\"entry\":\""
            << d.toISO ()
            << "\",\"description\":\""
            << json::encode (i->second)
            << "\"}";

        ++annotations_written;
      }
    }

    out << "]";
  }

  out << "}";
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
void Task::getAnnotations (std::map <std::string, std::string>& annotations) const
{
  annotations.clear ();

  Task::const_iterator ci;
  for (ci = this->begin (); ci != this->end (); ++ci)
    if (ci->first.substr (0, 11) == "annotation_")
      annotations.insert (*ci);
}

////////////////////////////////////////////////////////////////////////////////
void Task::setAnnotations (const std::map <std::string, std::string>& annotations)
{
  // Erase old annotations.
  removeAnnotations ();

  std::map <std::string, std::string>::const_iterator ci;
  for (ci = annotations.begin (); ci != annotations.end (); ++ci)
    this->insert (*ci);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
// The timestamp is part of the name:
//    annotation_1234567890:"..."
//
// Note that the time is incremented (one second) in order to find a unique
// timestamp.
void Task::addAnnotation (const std::string& description)
{
  time_t now = time (NULL);
  std::string key;

  do
  {
    key = "annotation_" + format ((int) now);
    ++now;
  }
  while (has (key));

  (*this)[key] = description;
  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeAnnotations ()
{
  // Erase old annotations.
  Task::iterator i = this->begin ();
  while (i != this->end ())
  {
    if (i->first.substr (0, 11) == "annotation_")
      this->erase (i++);
    else
      i++;
  }

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::addDependency (int id)
{
  if (id == this->id)
    throw std::string (STRING_TASK_DEPEND_ITSELF);

  // Check that id is resolvable.
  std::string uuid = context.tdb2.pending.uuid (id);
  if (uuid == "")
    throw format (STRING_TASK_DEPEND_MISSING, id);

  // Store the dependency.
  std::string depends = get ("depends");
  if (depends != "")
  {
    // Check for extant dependency.
    if (depends.find (uuid) == std::string::npos)
      set ("depends", depends + "," + uuid);
    else
      throw format (STRING_TASK_DEPEND_DUP, this->id, id);
  }
  else
    set ("depends", uuid);

  // Prevent circular dependencies.
  if (dependencyIsCircular (*this))
    throw std::string (STRING_TASK_DEPEND_CIRCULAR);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeDependency (const std::string& uuid)
{
  std::vector <std::string> deps;
  split (deps, get ("depends"), ',');

  std::vector <std::string>::iterator i;
  i = std::find (deps.begin (), deps.end (), uuid);
  if (i != deps.end ())
  {
    deps.erase (i);
    std::string combined;
    join (combined, ",", deps);
    set ("depends", combined);
    recalc_urgency = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeDependency (int id)
{
  std::string uuid = context.tdb2.pending.uuid (id);
  if (uuid != "")
    removeDependency (uuid);
  else
    throw std::string (STRING_TASK_DEPEND_NO_UUID);
}

////////////////////////////////////////////////////////////////////////////////
void Task::getDependencies (std::vector <int>& all) const
{
  std::vector <std::string> deps;
  split (deps, get ("depends"), ',');

  all.clear ();

  std::vector <std::string>::iterator i;
  for (i = deps.begin (); i != deps.end (); ++i)
    all.push_back (context.tdb2.pending.id (*i));
}

////////////////////////////////////////////////////////////////////////////////
void Task::getDependencies (std::vector <std::string>& all) const
{
  all.clear ();
  split (all, get ("depends"), ',');
}

////////////////////////////////////////////////////////////////////////////////
int Task::getTagCount () const
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ',');

  return (int) tags.size ();
}

////////////////////////////////////////////////////////////////////////////////
bool Task::hasTag (const std::string& tag) const
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ',');

  if (std::find (tags.begin (), tags.end (), tag) != tags.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Task::addTag (const std::string& tag)
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ',');

  if (std::find (tags.begin (), tags.end (), tag) == tags.end ())
  {
    tags.push_back (tag);
    std::string combined;
    join (combined, ",", tags);
    set ("tags", combined);

    recalc_urgency = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::addTags (const std::vector <std::string>& tags)
{
  remove ("tags");

  std::vector <std::string>::const_iterator it;
  for (it = tags.begin (); it != tags.end (); ++it)
    addTag (*it);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::getTags (std::vector<std::string>& tags) const
{
  split (tags, get ("tags"), ',');
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeTag (const std::string& tag)
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ',');

  std::vector <std::string>::iterator i;
  i = std::find (tags.begin (), tags.end (), tag);
  if (i != tags.end ())
  {
    tags.erase (i);
    std::string combined;
    join (combined, ",", tags);
    set ("tags", combined);
  }

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::substitute (
  const std::string& from,
  const std::string& to,
  bool global)
{
  // Get the data to modify.
  std::string description = get ("description");
  std::map <std::string, std::string> annotations;
  getAnnotations (annotations);

  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  // Count the changes, so we know whether to proceed to annotations, after
  // modifying description.
  int changes = 0;
  bool done = false;

  // Regex support is optional.
  if (context.config.getBoolean ("regex"))
  {
    // Create the regex.
    RX rx (from, sensitive);
    std::vector <int> start;
    std::vector <int> end;

    // Perform all subs on description.
    if (rx.match (start, end, description))
    {
      int skew = 0;
      for (unsigned int i = 0; i < start.size () && !done; ++i)
      {
        description.replace (start[i + skew], end[i] - start[i], to);
        skew += to.length () - (end[i] - start[i]);
        ++changes;

        if (!global)
          done = true;
      }
    }

    if (!done)
    {
      // Perform all subs on annotations.
      std::map <std::string, std::string>::iterator it;
      for (it = annotations.begin (); it != annotations.end () && !done; ++it)
      {
        start.clear ();
        end.clear ();
        if (rx.match (start, end, it->second))
        {
          int skew = 0;
          for (unsigned int i = 0; i < start.size () && !done; ++i)
          {
            it->second.replace (start[i + skew], end[i] - start[i], to);
            skew += to.length () - (end[i] - start[i]);
            ++changes;

            if (!global)
              done = true;
          }
        }
      }
    }
  }
  else
  {
    // Perform all subs on description.
    int counter = 0;
    std::string::size_type pos = 0;
    int skew = 0;

    while ((pos = ::find (description, from, pos, sensitive)) != std::string::npos && !done)
    {
      description.replace (pos + skew, from.length (), to);
      skew += to.length () - from.length ();

      pos += to.length ();
      ++changes;

      if (!global)
        done = true;

      if (++counter > APPROACHING_INFINITY)
        throw format (STRING_INFINITE_LOOP, APPROACHING_INFINITY);
    }

    if (!done)
    {
      // Perform all subs on annotations.
      counter = 0;
      std::map <std::string, std::string>::iterator i;
      for (i = annotations.begin (); i != annotations.end () && !done; ++i)
      {
        pos = 0;
        skew = 0;
        while ((pos = ::find (i->second, from, pos, sensitive)) != std::string::npos && !done)
        {
          i->second.replace (pos + skew, from.length (), to);
          skew += to.length () - from.length ();

          pos += to.length ();
          ++changes;

          if (!global)
            done = true;

          if (++counter > APPROACHING_INFINITY)
            throw format (STRING_INFINITE_LOOP, APPROACHING_INFINITY);
        }
      }
    }
  }

  if (changes)
  {
    set ("description", description);
    setAnnotations (annotations);
    recalc_urgency = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
// The purpose of Task::validate is three-fold:
//   1) To provide missing attributes where possible
//   2) To provide suitable warnings about odd states
//   3) To generate errors when the inconsistencies are not fixable
//
void Task::validate ()
{
  Task::status status = getStatus ();

  // 1) Provide missing attributes where possible
  // Provide a UUID if necessary.
  if (! has ("uuid"))
    set ("uuid", uuid ());

  // Recurring tasks get a special status.
  if (status == Task::pending &&
      has ("due")             &&
      has ("recur")           &&
      ! has ("parent"))
    status = Task::recurring;

  // Tasks with a wait: date get a special status.
  else if (status == Task::pending &&
           has ("wait"))
    status = Task::waiting;

  // By default, tasks are pending.
  else if (! has ("status"))
    status = Task::pending;

  // Store the derived status.
  setStatus (status);

  // Provide an entry date unless user already specified one.
  if (!has ("entry"))
    setEntry ();

  // Completed tasks need an end date, so inherit the entry date.
  if (! has ("end") &&
      (getStatus () == Task::completed ||
       getStatus () == Task::deleted))
    setEnd ();

  // Override with default.project, if not specified.
  if (! has ("project"))
  {
    std::string defaultProject = context.config.get ("default.project");
    if (defaultProject != "" &&
        context.columns["project"]->validate (defaultProject))
      set ("project", defaultProject);
  }

  // Override with default.priority, if not specified.
  if (get ("priority") == "")
  {
    std::string defaultPriority = context.config.get ("default.priority");
    if (defaultPriority != "" &&
        context.columns["priority"]->validate (defaultPriority))
      set ("priority", defaultPriority);
  }

  // Override with default.due, if not specified.
  if (get ("due") == "")
  {
    std::string defaultDue = context.config.get ("default.due");
    if (defaultDue != "" &&
        context.columns["due"]->validate (defaultDue))
      set ("due", Date (defaultDue).toEpoch ());
  }

  // 2) To provide suitable warnings about odd states

  // When a task has a due date, other dates should conform.
  if (has ("due"))
  {
    Date due (get_date ("due"));

    // Verify wait < due
    if (has ("wait"))
    {
      Date wait (get_date ("wait"));
      if (wait > due)
        context.footnote (STRING_TASK_VALID_WAIT);
    }

    Date entry (get_date ("entry"));

    if (has ("start"))
    {
      Date start (get_date ("start"));
      if (entry > start)
        context.footnote (STRING_TASK_VALID_START);
    }

    if (has ("end"))
    {
      Date end (get_date ("end"));
      if (entry > end)
        context.footnote (STRING_TASK_VALID_END);
    }
  }

  // 3) To generate errors when the inconsistencies are not fixable

  // There is no fixing a missing description.
  if (!has ("description"))
    throw std::string (STRING_TASK_VALID_DESC);
  else if (get ("description") == "")
    throw std::string (STRING_TASK_VALID_BLANK);

  // Cannot have a recur frequency with no due date - when would it recur?
  if (! has ("due") && has ("recur"))
    throw std::string (STRING_TASK_VALID_REC_DUE);

  // Cannot have an until date no recurrence frequency.
  if (has ("until") && !has ("recur"))
    throw std::string (STRING_TASK_VALID_UNTIL);

  // Recur durations must be valid.
  if (has ("recur"))
  {
    Duration d;
    if (! d.valid (get ("recur")))
      throw std::string (format (STRING_TASK_VALID_RECUR, get ("recur")));
  }

  // Priorities must be valid.
  if (has ("priority"))
  {
    std::string priority = get ("priority");
    if (priority != "H" &&
        priority != "M" &&
        priority != "L")
      throw format (STRING_TASK_VALID_PRIORITY, priority);
  }
}

////////////////////////////////////////////////////////////////////////////////
int Task::determineVersion (const std::string& line)
{
  // Version 2 looks like:
  //
  //   uuid status [tags] [attributes] description\n
  //
  // Where uuid looks like:
  //
  //   27755d92-c5e9-4c21-bd8e-c3dd9e6d3cf7
  //
  // Scan for the hyphens in the uuid, the following space, and a valid status
  // character.
  if (line[8]  == '-' &&
      line[13] == '-' &&
      line[18] == '-' &&
      line[23] == '-' &&
      line[36] == ' ' &&
      (line[37] == '-' || line[37] == '+' || line[37] == 'X' || line[37] == 'r'))
  {
    // Version 3 looks like:
    //
    //   uuid status [tags] [attributes] [annotations] description\n
    //
    // Scan for the number of [] pairs.
    std::string::size_type tagAtts  = line.find ("] [", 0);
    std::string::size_type attsAnno = line.find ("] [", tagAtts + 1);
    std::string::size_type annoDesc = line.find ("] ",  attsAnno + 1);
    if (tagAtts  != std::string::npos &&
        attsAnno != std::string::npos &&
        annoDesc != std::string::npos)
      return 3;
    else
      return 2;
  }

  // Version 4 looks like:
  //
  //   [name:"value" ...]
  //
  // Scan for [, ] and :".
  else if (line[0] == '[' &&
           line[line.length () - 1] == ']' &&
           line.find ("uuid:\"") != std::string::npos)
    return 4;

  // Version 1 looks like:
  //
  //   [tags] [attributes] description\n
  //   X [tags] [attributes] description\n
  //
  // Scan for the first character being either the bracket or X.
  else if (line.find ("X [") == 0 ||
           line.find ("uuid") == std::string::npos ||
           (line[0] == '[' &&
            line.substr (line.length () - 1, 1) != "]"))
    return 1;

  // Version 5?
  //
  // Fortunately, with the hindsight that will come with version 5, the
  // identifying characteristics of 1, 2, 3 and 4 may be modified such that if 5
  // has a UUID followed by a status, then there is still a way to differentiate
  // between 2, 3, 4 and 5.
  //
  // The danger is that a version 3 binary reads and misinterprets a version 4
  // file.  This is why it is a good idea to rely on an explicit version
  // declaration rather than chance positioning.

  // Zero means 'no idea'.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Urgency is defined as a polynomial, the value of which is calculated in this
// function, according to:
//
//   U = A.t  + B.t  + C.t  ...
//          a      b      c
//
//   U       = urgency
//   A       = coefficient for term a
//   t sub a = numeric scale from 0 -> 1, with 1 being the highest
//             urgency, derived from one task attribute and mapped
//             to the numeric scale
//
// See rfc31-urgency.txt for full details.
//
float Task::urgency_c () const
{
  float value = 0.0;
  value += urgency_priority ()    * context.config.getReal ("urgency.priority.coefficient");
  value += urgency_project ()     * context.config.getReal ("urgency.project.coefficient");
  value += urgency_active ()      * context.config.getReal ("urgency.active.coefficient");
  value += urgency_waiting ()     * context.config.getReal ("urgency.waiting.coefficient");
  value += urgency_blocked ()     * context.config.getReal ("urgency.blocked.coefficient");
  value += urgency_annotations () * context.config.getReal ("urgency.annotations.coefficient");
  value += urgency_tags ()        * context.config.getReal ("urgency.tags.coefficient");
  value += urgency_next ()        * context.config.getReal ("urgency.next.coefficient");
  value += urgency_due ()         * context.config.getReal ("urgency.due.coefficient");
  value += urgency_blocking ()    * context.config.getReal ("urgency.blocking.coefficient");
  value += urgency_age ()         * context.config.getReal ("urgency.age.coefficient");

/* 
  std::cout << "# Urgency for " << id << ":\n"
          << "#     pri " << (urgency_priority ()    * context.config.getReal ("urgency.priority.coefficient"))
          << "#     pro " << (urgency_project ()     * context.config.getReal ("urgency.project.coefficient"))
          << "#     act " << (urgency_active ()      * context.config.getReal ("urgency.active.coefficient"))
          << "#     wai " << (urgency_waiting ()     * context.config.getReal ("urgency.waiting.coefficient"))
          << "#     blk " << (urgency_blocked ()     * context.config.getReal ("urgency.blocked.coefficient"))
          << "#     ann " << (urgency_annotations () * context.config.getReal ("urgency.annotations.coefficient"))
          << "#     tag " << (urgency_tags ()        * context.config.getReal ("urgency.tags.coefficient"))
          << "#     nex " << (urgency_next ()        * context.config.getReal ("urgency.next.coefficient"))
          << "#     due " << (urgency_due ()         * context.config.getReal ("urgency.due.coefficient"))
          << "#     bkg " << (urgency_blocking ()    * context.config.getReal ("urgency.blocking.coefficient"))
          << "#     age " << (urgency_age ()         * context.config.getReal ("urgency.age.coefficient"));
*/

  // Tag- and project-specific coefficients.
  std::vector <std::string> all;
  context.config.all (all);

  std::vector <std::string>::iterator var;
  for (var = all.begin (); var != all.end (); ++var)
  {
    if (var->substr (0, 13) == "urgency.user.")
    {
      // urgency.user.project.<project>.coefficient
      std::string::size_type end = std::string::npos;
      if (var->substr (13, 8) == "project." &&
          (end = var->find (".coefficient")) != std::string::npos)
      {
        std::string project = var->substr (21, end - 21);

        if (get ("project").find (project) == 0)
          value += context.config.getReal (*var);
      }

      // urgency.user.tag.<tag>.coefficient
      if (var->substr (13, 4) == "tag." &&
          (end = var->find (".coefficient")) != std::string::npos)
      {
        std::string tag = var->substr (17, end - 17);

        if (hasTag (tag))
          value += context.config.getReal (*var);
      }
    }
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency ()
{
  if (recalc_urgency)
  {
    urgency_value = urgency_c ();

    // Return the sum of all terms.
    recalc_urgency = false;
  }

  return urgency_value;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_priority () const
{
  std::string value = get ("priority");

       if (value == "H") return 1.0;
  else if (value == "M") return 0.65;
  else if (value == "L") return 0.3;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_project () const
{
  if (has ("project"))
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_active () const
{
  if (has ("start"))
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_waiting () const
{
  if (get ("status") == "waiting")
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_blocked () const
{
  if (has ("depends"))
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_annotations () const
{
  std::map <std::string, std::string> annos;
  getAnnotations (annos);

       if (annos.size () >= 3) return 1.0;
  else if (annos.size () == 2) return 0.9;
  else if (annos.size () == 1) return 0.8;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_tags () const
{
  switch (getTagCount ())
  {
  case 0:  return 0.0;
  case 1:  return 0.8;
  case 2:  return 0.9;
  default: return 1.0;
  }
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_next () const
{
  if (hasTag ("next"))
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_due () const
{
  if (has ("due"))
  {
    Date now;
    Date due (get_date ("due"));
    int days_overdue = (now - due) / 86400;

         if (days_overdue >=  7)  return 1.0;
    else if (days_overdue >=  6)  return 0.96;
    else if (days_overdue >=  5)  return 0.92;
    else if (days_overdue >=  4)  return 0.88;
    else if (days_overdue >=  3)  return 0.84;
    else if (days_overdue >=  2)  return 0.80;
    else if (days_overdue >=  1)  return 0.76;
    else if (days_overdue >=  0)  return 0.72;
    else if (days_overdue >= -1)  return 0.68;
    else if (days_overdue >= -2)  return 0.64;
    else if (days_overdue >= -3)  return 0.60;
    else if (days_overdue >= -4)  return 0.56;
    else if (days_overdue >= -5)  return 0.52;
    else if (days_overdue >= -6)  return 0.48;
    else if (days_overdue >= -7)  return 0.44;
    else if (days_overdue >= -8)  return 0.40;
    else if (days_overdue >= -9)  return 0.36;
    else if (days_overdue >= -10) return 0.32;
    else if (days_overdue >= -11) return 0.28;
    else if (days_overdue >= -12) return 0.24;
    else if (days_overdue >= -13) return 0.20;
    else                          return 0.16;
  }

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_age () const
{
  if (has ("entry"))
  {
    Date now;
    Date entry (get_date ("entry"));
    int   age  = (now - entry) / 86400;  // in days
    float max  = context.config.getReal ("urgency.age.max");

    if (max == 0)
      return 1.0;

    if (age > max)
      return 1.0;
    else
      return (1.0 * age/max);
  }

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_blocking () const
{
  if (dependencyIsBlocking (*this))
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
