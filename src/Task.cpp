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
#include <algorithm>
#include <Context.h>
#include <Nibbler.h>
#include <Date.h>
#include <Duration.h>
#include <Task.h>
#include <JSON.h>
#include <text.h>
#include <util.h>
#include <main.h>

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
: Record (other)
, id (other.id)
{
}

////////////////////////////////////////////////////////////////////////////////
Task& Task::operator= (const Task& other)
{
  if (this != &other)
  {
    Record::operator= (other);
    id = other.id;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// The uuid and id attributes must be exempt for comparison.
bool Task::operator== (const Task& other)
{
  if (size () != other.size ())
    return false;

  foreach (att, *this)
    if (att->second.name () != "uuid")
      if (att->second.value () != other.get (att->second.name ()))
        return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
Task::Task (const std::string& input)
{
  id = 0;
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Task::~Task ()
{
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::textToStatus (const std::string& input)
{
       if (input == "pending")   return Task::pending;    // TODO i18n
  else if (input == "completed") return Task::completed;  // TODO i18n
  else if (input == "deleted")   return Task::deleted;    // TODO i18n
  else if (input == "recurring") return Task::recurring;  // TODO i18n
  else if (input == "waiting")   return Task::waiting;    // TODO i18n

  return Task::pending;
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::statusToText (Task::status s)
{
       if (s == Task::pending)   return "pending";        // TODO i18n
  else if (s == Task::completed) return "completed";      // TODO i18n
  else if (s == Task::deleted)   return "deleted";        // TODO i18n
  else if (s == Task::recurring) return "recurring";      // TODO i18n
  else if (s == Task::waiting)   return "waiting";        // TODO i18n

  return "pending";
}

////////////////////////////////////////////////////////////////////////////////
void Task::setEntry ()
{
  char entryTime[16];
  sprintf (entryTime, "%u", (unsigned int) time (NULL));
  set ("entry", entryTime); // No i18n

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::getStatus () const
{
  return textToStatus (get ("status")); // No i18n
}

////////////////////////////////////////////////////////////////////////////////
void Task::setStatus (Task::status status)
{
  set ("status", statusToText (status)); // No i18n

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
// Attempt an FF4 parse first, using Record::parse, and in the event of an error
// try a legacy parse (F3, FF2).  Note that FF1 is no longer supported.
void Task::parse (const std::string& line)
{
  std::string copy;
  if (line[line.length () - 1] == '\n')
    copy = line.substr (0, line.length () - 1);
  else
    copy = line;

  try
  {
    Record::parse (copy);
  }

  catch (std::string& e)
  {
    legacyParse (copy);
  }
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
    throw std::string ("Taskwarrior no longer supports file format 1, originally used "
                       "between 27 November 2006 and 31 December 2007.");  // TODO i18n
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

        set ("status", statusToText (status)); // No i18n

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

            set ("description", line.substr (closeAttrBracket + 2)); // No i18n
          }
          else
            throw std::string ("Missing attribute brackets"); // TODO i18n
        }
        else
          throw std::string ("Missing tag brackets"); // TODO i18n
      }
      else
        throw std::string ("Line too short"); // TODO i18n

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

        set ("status", statusToText (status)); // No i18n

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
                  set ("annotation_" + name, value); // No i18n
                }
              }

              set ("description", line.substr (closeAnnoBracket + 2)); // No i18n
            }
            else
              throw std::string ("Missing annotation brackets."); // TODO i18n
          }
          else
            throw std::string ("Missing attribute brackets."); // TODO i18n
        }
        else
          throw std::string ("Missing tag brackets."); // TODO i18n
      }
      else
        throw std::string ("Line too short."); // TODO i18n
    }
    break;

  default:
    throw std::string ("Unrecognized taskwarrior file format."); // TODO i18n
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::composeCSV () const
{
  std::stringstream out;

  // Deliberately no 'id'.
  out << "'" << get ("uuid")     << "',"; // No i18n
  out << "'" << get ("status")   << "',"; // No i18n

  // Tags
  std::vector <std::string> tags;
  getTags (tags);
  std::string allTags;
  join (allTags, " ", tags);              // No i18n
  out << "'" << allTags          << "',"; // No i18n

  out <<        get ("entry")    <<  ","; // No i18n
  out <<        get ("start")    <<  ","; // No i18n

  if (has ("due"))
    out <<      get ("due")      <<  ","; // No i18n
  else
    out << ","; // No i18n

  if (has ("recur"))
    out <<      get ("recur")    <<  ","; // No i18n
  else
    out << ","; // No i18n

  out <<        get ("end")      <<  ","; // No i18n

  if (has ("project"))
    out << "'" << get ("project")  << "',"; // No i18n
  else
    out << ","; // No i18n

  if (has ("priority"))
    out << "'" << get ("priority") << "',"; // No i18n
  else
    out << ","; // No i18n

  if (has ("fg"))
    out << "'" << get ("fg")     << "',"; // No i18n
  else
    out << ","; // No i18n

  if (has ("bg"))
    out << "'" << get ("bg")     << "',"; // No i18n
  else
    out << ","; // No i18n

  // Convert single quotes to double quotes, because single quotes are used to
  // delimit the values that need it.
  std::string clean = get ("description"); // No i18n
  std::replace (clean.begin (), clean.end (), '\'', '"'); // No i18n
  out << "'" << clean            << "'\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::composeYAML () const
{
  std::stringstream out;

  // Task header.
  out << "  task:\n";

  // Get all the supported attribute names.
  std::vector <std::string> names;
  Att::allNames (names);
  std::sort (names.begin (), names.end ());

  // Only include non-trivial attributes.
  std::string value;
  foreach (name, names)
    if ((value = get (*name)) != "")
      out << "    " << *name << ": " << value << "\n";

  // Now the annotations, which are not listed by the Att::allNames call.
  std::vector <Att> annotations;
  getAnnotations (annotations);
  foreach (a, annotations)
    out << "    annotation:\n"
        << "      entry: "       << a->name().substr (11) << "\n"
        << "      description: " << a->value ()           << "\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::composeJSON (bool include_id /*= false*/) const
{
  std::stringstream out;
  out << "{";

  // ID inclusion is optional, not recommended.
  if (include_id)
    out << "\"id\":" << id << ",";

  // Used for determining type.
  Att att;

  // First the non-annotations.
  int attributes_written = 0;
  int annotation_count = 0;
  std::map <std::string, Att>::const_iterator i;
  for (i = this->begin (); i != this->end (); ++i)
  {
    if (attributes_written)
      out << ",";

    // Annotations are simply counted.
    if (i->second.name ().substr (0, 11) == "annotation_")
    {
      ++annotation_count;
    }

    // Date fields are written as ISO 8601.
    else if (att.type (i->second.name ()) == "date")
    {
      Date d (i->second.value ());
      out << "\""
          << i->second.name ()
          << "\":\""
          << d.toISO ()
          << "\"";

      ++attributes_written;
    }

    // Tags are converted to an array.
    else if (i->second.name () == "tags")
    {
      std::vector <std::string> tags;
      split (tags, i->second.value (), ',');

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
          << i->second.name ()
          << "\":\""
          << JSON::encode (i->second.value ())
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
      if (i->second.name ().substr (0, 11) == "annotation_")
      {
        if (annotations_written)
          out << ",";

        Date d (i->second.name ().substr (11));
        out << "{\"entry\":\""
            << d.toISO ()
            << "\",\"description\":\""
            << JSON::encode (i->second.value ())
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
void Task::getAnnotations (std::vector <Att>& annotations) const
{
  annotations.clear ();

  Record::const_iterator ci;
  for (ci = this->begin (); ci != this->end (); ++ci)
    if (ci->first.substr (0, 11) == "annotation_")  // No i18n
      annotations.push_back (ci->second);
}

////////////////////////////////////////////////////////////////////////////////
void Task::setAnnotations (const std::vector <Att>& annotations)
{
  // Erase old annotations.
  removeAnnotations ();

  std::vector <Att>::const_iterator ci;
  for (ci = annotations.begin (); ci != annotations.end (); ++ci)
    (*this)[ci->name ()] = *ci;

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
// The timestamp is part of the name:
//    annotation_1234567890:"..."
//
void Task::addAnnotation (const std::string& description)
{
  std::stringstream s;
  s << "annotation_" << time (NULL); // No i18n

  (*this)[s.str ()] = Att (s.str (), description);
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeAnnotations ()
{
  // Erase old annotations.
  Record::iterator i = this->begin ();
  while (i != this->end ())
  {
    if (i->first.substr (0, 11) == "annotation_") // No i18n
      this->erase (i++);
    else
      i++;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::addDependency (int id)
{
  if (id == this->id)
    throw std::string ("A task cannot be dependent on itself.");

  // Check for extant dependency.
  std::string uuid = context.tdb.uuid (id);
  if (uuid == "")
  {
    std::stringstream s;
    s << "Could not create a dependency on task " << id << " - not found.";
    throw s.str ();
  }

  // Store the dependency.
  std::string depends = get ("depends");
  if (depends.length ())
  {
    if (depends.find (uuid) == std::string::npos)
      set ("depends", depends + "," + uuid);
    else
    {
      std::stringstream out;
      out << "Task " << this->id << " already depends on task " << id << ".";
      throw out.str ();
    }
  }
  else
    set ("depends", uuid);

  // Prevent circular dependencies.
  if (dependencyIsCircular (*this))
    throw std::string ("Circular dependency detected and disallowed.");
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
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeDependency (int id)
{
  std::string uuid = context.tdb.uuid (id);
  if (uuid != "")
    removeDependency (uuid);
  else
  {
    std::stringstream s;
    s << "Could not find a UUID for id " << id << ".";
    throw s.str ();
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::getDependencies (std::vector <int>& all) const
{
  std::vector <std::string> deps;
  split (deps, get ("depends"), ',');

  all.clear ();

  std::vector <std::string>::iterator i;
  for (i = deps.begin (); i != deps.end (); ++i)
    all.push_back (context.tdb.id (*i));
}

////////////////////////////////////////////////////////////////////////////////
void Task::getDependencies (std::vector <std::string>& all) const
{
  all.clear ();
  split (all, get ("depends"), ',');
}

////////////////////////////////////////////////////////////////////////////////
int Task::getTagCount ()
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ','); // No i18n

  return (int) tags.size ();
}

////////////////////////////////////////////////////////////////////////////////
bool Task::hasTag (const std::string& tag)
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ','); // No i18n

  if (std::find (tags.begin (), tags.end (), tag) != tags.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Task::addTag (const std::string& tag)
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ','); // No i18n

  if (std::find (tags.begin (), tags.end (), tag) == tags.end ())
  {
    tags.push_back (tag);
    std::string combined;
    join (combined, ",", tags); // No i18n
    set ("tags", combined); // No i18n
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::addTags (const std::vector <std::string>& tags)
{
  remove ("tags"); // No i18n

  std::vector <std::string>::const_iterator it;
  for (it = tags.begin (); it != tags.end (); ++it)
    addTag (*it);
}

////////////////////////////////////////////////////////////////////////////////
void Task::getTags (std::vector<std::string>& tags) const
{
  split (tags, get ("tags"), ','); // No i18n
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeTag (const std::string& tag)
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ','); // No i18n

  std::vector <std::string>::iterator i;
  i = std::find (tags.begin (), tags.end (), tag);
  if (i != tags.end ())
  {
    tags.erase (i);
    std::string combined;
    join (combined, ",", tags); // No i18n
    set ("tags", combined); // No i18n
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::validate () const
{
  // Every task needs an ID, entry and description attribute.
  if (!has ("uuid")  ||
      !has ("entry") ||
      !has ("description"))
    throw std::string ("A task must have a description in order to be valid.");

  if (get ("description") == "") // No i18n
    throw std::string ("Cannot add a task that is blank, or contains <CR> or <LF> characters.");

  if (has ("due"))
  {
    Date due (::atoi (get ("due").c_str ()));

    // Verify wait < due
    if (has ("wait"))
    {
      Date wait (::atoi (get ("wait").c_str ()));
      if (wait > due)
        throw std::string ("A 'wait' date must be before a 'due' date.");
    }

    Date entry (::atoi (get ("entry").c_str ()));

    if (has ("start"))
    {
      Date start (::atoi (get ("start").c_str ()));
      if (entry > start)
        throw std::string ("A 'start' date must be after an 'entry' date.");
    }

    if (has ("end"))
    {
      Date end (::atoi (get ("end").c_str ()));
      if (entry > end)
        throw std::string ("An 'end' date must be after an 'entry' date.");
    }
  }
  else
  {
    if (has ("recur"))
      throw std::string ("You cannot specify a recurring task without a due date.");
  }

  if (has ("until") && !has ("recur"))
    throw std::string ("You cannot specify an until date for a non-recurring task.");

  // Recur durations must be valid.
  if (has ("recur"))
  {
    Duration d;
    if (! d.valid (get ("recur")))
      throw std::string ("A recurrence value must be valid.");
  }

  if (has ("wait") &&
      getStatus () == Task::recurring)
    throw std::string ("You cannot create a task that is both waiting and recurring.");
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
    std::string::size_type tagAtts  = line.find ("] [", 0); // No i18n
    std::string::size_type attsAnno = line.find ("] [", tagAtts + 1); // No i18n
    std::string::size_type annoDesc = line.find ("] ",  attsAnno + 1); // No i18n
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
           line.find ("uuid:\"") != std::string::npos) // No i18n
    return 4;

  // Version 1 looks like:
  //
  //   [tags] [attributes] description\n
  //   X [tags] [attributes] description\n
  //
  // Scan for the first character being either the bracket or X.
  else if (line.find ("X [") == 0 ||
           line.find ("uuid") == std::string::npos || // No i18n
           (line[0] == '[' &&
            line.substr (line.length () - 1, 1) != "]")) // No i18n
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
float Task::urgency ()
{
  if (! recalc_urgency)
    return urgency_value;

  // urgency.priority.coefficient
  float coefficient = context.config.getReal ("urgency.priority.coefficient");
  float term;

  std::string value = get ("priority");
       if (value == "H") term = 1.0;
  else if (value == "M") term = 0.65;
  else if (value == "L") term = 0.3;
  else                   term = 0.0;

  urgency_value += term * coefficient;

  // urgency.project.coefficient
  coefficient = context.config.getReal ("urgency.project.coefficient");

  value = get ("project");
  if (value != "") term = 1.0;
  else             term = 0.0;

  urgency_value += term * coefficient;

  // urgency.active.coefficient
  coefficient = context.config.getReal ("urgency.active.coefficient");

  value = get ("start");
  if (value != "") term = 1.0;
  else             term = 0.0;

  urgency_value += term * coefficient;

  // urgency.waiting.coefficient
  coefficient = context.config.getReal ("urgency.waiting.coefficient");

  value = get ("status");
       if (value == "pending") term = 1.0;
  else if (value == "waiting") term = 0.0;

  urgency_value += term * coefficient;

  // urgency.blocked.coefficient
  coefficient = context.config.getReal ("urgency.blocked.coefficient");

  value = get ("depends");
  if (value != "") term = 1.0;
  else             term = 0.0;

  urgency_value += term * coefficient;

  // urgency.annotations.coefficient
  coefficient = context.config.getReal ("urgency.annotations.coefficient");

  std::vector <Att> annos;
  getAnnotations (annos);
       if (annos.size () >= 3) term = 1.0;
  else if (annos.size () == 2) term = 0.9;
  else if (annos.size () == 1) term = 0.8;
  else                         term = 0.0;

  urgency_value += term * coefficient;

  // urgency.tags.coefficient
  coefficient = context.config.getReal ("urgency.tags.coefficient");

  int count = getTagCount ();
       if (count >= 3) term = 1.0;
  else if (count == 2) term = 0.9;
  else if (count == 1) term = 0.8;
  else                 term = 0.0;

  urgency_value += term * coefficient;

  // urgency.next.coefficient
  coefficient = context.config.getReal ("urgency.next.coefficient");

  if (hasTag ("next")) term = 1.0;
  else                 term = 0.0;

  urgency_value += term * coefficient;

  // urgency.due.coefficient
  // overdue days 7 -> 1.0
  //              6 -> 0.96
  //              5 -> 0.92
  //              5 -> 0.88
  //              5 -> 0.84
  //              5 -> 0.80
  //              5 -> 0.76
  //              0 -> 0.72
  //             -1 -> 0.68
  //             -2 -> 0.64
  //             -3 -> 0.60
  //             -4 -> 0.56
  //             -5 -> 0.52
  //             -6 -> 0.48
  //             -7 -> 0.44
  //             -8 -> 0.40
  //             -9 -> 0.36
  //            -10 -> 0.32
  //            -11 -> 0.28
  //            -12 -> 0.24
  //            -13 -> 0.20
  //            -14 -> 0.16
  //    no due date -> 0.0
  coefficient = context.config.getReal ("urgency.due.coefficient");
  if (has ("due"))
  {
    Date now;
    Date due (get ("due"));
    int days_overdue = (now - due) / 86400;

         if (days_overdue >=  7)  term = 1.0;
    else if (days_overdue >=  6)  term = 0.96;
    else if (days_overdue >=  5)  term = 0.92;
    else if (days_overdue >=  4)  term = 0.88;
    else if (days_overdue >=  3)  term = 0.84;
    else if (days_overdue >=  2)  term = 0.80;
    else if (days_overdue >=  1)  term = 0.76;
    else if (days_overdue >=  0)  term = 0.72;
    else if (days_overdue >= -1)  term = 0.68;
    else if (days_overdue >= -2)  term = 0.64;
    else if (days_overdue >= -3)  term = 0.60;
    else if (days_overdue >= -4)  term = 0.56;
    else if (days_overdue >= -5)  term = 0.52;
    else if (days_overdue >= -6)  term = 0.48;
    else if (days_overdue >= -7)  term = 0.44;
    else if (days_overdue >= -8)  term = 0.40;
    else if (days_overdue >= -9)  term = 0.36;
    else if (days_overdue >= -10) term = 0.32;
    else if (days_overdue >= -11) term = 0.28;
    else if (days_overdue >= -12) term = 0.24;
    else if (days_overdue >= -13) term = 0.20;
    else                          term = 0.16;
  }
  else
    term = 0.0;

  urgency_value += term * coefficient;

  // Tag- and project-specific coefficients.
  std::vector <std::string> all;
  context.config.all (all);

  foreach (var, all)
  {
    if (var->substr (0, 13) == "urgency.user.")
    {
      // urgency.user.project.<project>.coefficient
      std::string::size_type end = std::string::npos;
      if (var->substr (13, 8) == "project." &&
          (end = var->find (".coefficient")) != std::string::npos)
      {
        std::string project = var->substr (21, end - 21);
        coefficient = context.config.getReal (*var);

        if (get ("project").find (project) == 0)
          urgency_value += coefficient;
      }

    // urgency.user.tag.<tag>.coefficient
      if (var->substr (13, 4) == "tag." &&
          (end = var->find (".coefficient")) != std::string::npos)
      {
        std::string tag = var->substr (17, end - 17);
        coefficient = context.config.getReal (*var);

        if (hasTag (tag))
          urgency_value += coefficient;
      }
    }
  }

  // urgency.blocking.coefficient
  coefficient = context.config.getReal ("urgency.blocking.coefficient");

  if (dependencyIsBlocking (*this)) term = 1.0;
  else                              term = 0.0;

  urgency_value += term * coefficient;

  // Return the sum of all terms.
  recalc_urgency = false;
  return urgency_value;
}

////////////////////////////////////////////////////////////////////////////////
