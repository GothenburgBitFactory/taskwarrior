////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2020, Paul Beckingham, Federico Hernandez.
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
#include <Task.h>
#include <sstream>
#include <cstdlib>
#include <cassert>
#include <string>
#ifdef PRODUCT_TASKWARRIOR
#include <cmath>
#include <cctype>
#endif
#include <cfloat>
#include <algorithm>
#include <Lexer.h>
#ifdef PRODUCT_TASKWARRIOR
#include <Context.h>
#include <Pig.h>
#endif
#include <Duration.h>
#include <Datetime.h>
#ifdef PRODUCT_TASKWARRIOR
#include <RX.h>
#endif
#include <shared.h>
#include <format.h>
#include <util.h>

#ifdef PRODUCT_TASKWARRIOR
#include <main.h>

#include <Eval.h>
#include <Variant.h>
#include <Filter.h>


#define APPROACHING_INFINITY 1000   // Close enough.  This isn't rocket surgery.

extern Task& contextTask;

static const float epsilon = 0.000001;
#endif

std::string Task::defaultProject;
std::string Task::defaultDue;
std::string Task::defaultScheduled;
bool Task::searchCaseSensitive     = true;
bool Task::regex                   = false;
std::map <std::string, std::string> Task::attributes;

std::map <std::string, float> Task::coefficients;
float Task::urgencyProjectCoefficient     = 0.0;
float Task::urgencyActiveCoefficient      = 0.0;
float Task::urgencyScheduledCoefficient   = 0.0;
float Task::urgencyWaitingCoefficient     = 0.0;
float Task::urgencyBlockedCoefficient     = 0.0;
float Task::urgencyAnnotationsCoefficient = 0.0;
float Task::urgencyTagsCoefficient        = 0.0;
float Task::urgencyDueCoefficient         = 0.0;
float Task::urgencyBlockingCoefficient    = 0.0;
float Task::urgencyAgeCoefficient         = 0.0;
float Task::urgencyAgeMax                 = 0.0;

std::map <std::string, std::vector <std::string>> Task::customOrder;

static const std::string dummy;

////////////////////////////////////////////////////////////////////////////////
// The uuid and id attributes must be exempt from comparison.
//
// This performs two tests which are sufficient and necessary for Task
// object equality (neglecting uuid and id):
//     - The attribute set sizes are the same
//     - For each attribute in the first set, there exists a same
//       attribute with a same value in the second set
//
// These two conditions are necessary. They are also sufficient, since there
// can be no extra data attribute in the second set, due to the same attribute
// set sizes.
bool Task::operator== (const Task& other)
{
  if (data.size () != other.data.size ())
    return false;

  for (const auto& i : data)
    if (i.first != "uuid" &&
        i.second != other.get (i.first))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
Task::Task (const std::string& input)
{
  id               = 0;
  urgency_value    = 0.0;
  recalc_urgency   = true;
  is_blocked       = false;
  is_blocking      = false;
  annotation_count = 0;

  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Task::Task (const json::object* obj)
{
  id               = 0;
  urgency_value    = 0.0;
  recalc_urgency   = true;
  is_blocked       = false;
  is_blocking      = false;
  annotation_count = 0;

  parseJSON (obj);
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::textToStatus (const std::string& input)
{
       if (input[0] == 'p') return Task::pending;
  else if (input[0] == 'c') return Task::completed;
  else if (input[0] == 'd') return Task::deleted;
  else if (input[0] == 'r') return Task::recurring;
  else if (input[0] == 'w') return Task::waiting;

  throw format ("The status '{1}' is not valid.", input);
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::statusToText (Task::status s)
{
       if (s == Task::pending)   return "pending";
  else if (s == Task::recurring) return "recurring";
  else if (s == Task::waiting)   return "waiting";
  else if (s == Task::completed) return "completed";
  else if (s == Task::deleted)   return "deleted";

  return "pending";
}

////////////////////////////////////////////////////////////////////////////////
// Returns a proper handle to the task. Tasks should not be referenced by UUIDs
// as long as they have non-zero ID.
std::string Task::identifier (bool shortened /* = false */) const
{
  if (id != 0)
    return format (id);
  else if (shortened)
    return get ("uuid").substr (0, 8);
  else
    return get ("uuid");
}

////////////////////////////////////////////////////////////////////////////////
void Task::setAsNow (const std::string& att)
{
  char now[16];
  snprintf (now, 16, "%u", (unsigned int) time (nullptr));
  set (att, now);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::has (const std::string& name) const
{
  if (data.find (name) != data.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Task::all ()
{
  std::vector <std::string> all;
  for (const auto& i : data)
    all.push_back (i.first);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::get (const std::string& name) const
{
  auto i = data.find (name);
  if (i != data.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
const std::string& Task::get_ref (const std::string& name) const
{
  auto i = data.find (name);
  if (i != data.end ())
    return i->second;

  return dummy;
}

////////////////////////////////////////////////////////////////////////////////
int Task::get_int (const std::string& name) const
{
  auto i = data.find (name);
  if (i != data.end ())
    return strtol (i->second.c_str (), nullptr, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
unsigned long Task::get_ulong (const std::string& name) const
{
  auto i = data.find (name);
  if (i != data.end ())
    return strtoul (i->second.c_str (), nullptr, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::get_float (const std::string& name) const
{
  auto i = data.find (name);
  if (i != data.end ())
    return strtof (i->second.c_str (), nullptr);

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
time_t Task::get_date (const std::string& name) const
{
  auto i = data.find (name);
  if (i != data.end ())
    return (time_t) strtoul (i->second.c_str (), nullptr, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Task::set (const std::string& name, const std::string& value)
{
  data[name] = json::decode (value);

  if (! name.compare (0, 11, "annotation_", 11))
    ++annotation_count;

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::set (const std::string& name, int value)
{
  data[name] = format (value);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::remove (const std::string& name)
{
  if (data.erase (name))
    recalc_urgency = true;

  if (! name.compare (0, 11, "annotation_", 11))
    --annotation_count;
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::getStatus () const
{
  if (! has ("status"))
    return Task::pending;

  return textToStatus (get ("status"));
}

////////////////////////////////////////////////////////////////////////////////
void Task::setStatus (Task::status status)
{
  set ("status", statusToText (status));

  recalc_urgency = true;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
// Determines status of a date attribute.
Task::dateState Task::getDateState (const std::string& name) const
{
  std::string value = get (name);
  if (value.length ())
  {
    Datetime reference (value);
    Datetime now;
    Datetime today ("today");

    if (reference < today)
      return dateBeforeToday;

    if (reference.sameDay (now))
    {
      if (reference < now)
        return dateEarlierToday;
      else
        return dateLaterToday;
    }

    int imminentperiod = Context::getContext ().config.getInteger ("due");
    if (imminentperiod == 0)
      return dateAfterToday;

    Datetime imminentDay = today + imminentperiod * 86400;
    if (reference < imminentDay)
      return dateAfterToday;
  }

  return dateNotDue;
}

////////////////////////////////////////////////////////////////////////////////
// Ready means pending, not blocked and either not scheduled or scheduled before
// now.
bool Task::is_ready () const
{
  return getStatus () == Task::pending &&
         ! is_blocked                  &&
         (! has ("scheduled")          ||
          Datetime ("now").operator> (get_date ("scheduled")));
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_due () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted)
    {
      Task::dateState state = getDateState ("due");
      if (state == dateAfterToday   ||
          state == dateEarlierToday ||
          state == dateLaterToday)
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_dueyesterday () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted)
    {
      if (Datetime ("yesterday").sameDay (get_date ("due")))
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_duetoday () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted)
    {
      Task::dateState state = getDateState ("due");
      if (state == dateEarlierToday ||
          state == dateLaterToday)
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_duetomorrow () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted)
    {
      if (Datetime ("tomorrow").sameDay (get_date ("due")))
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_dueweek () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted)
    {
      Datetime due (get_date ("due"));
      if (due >= Datetime ("sow") &&
          due <= Datetime ("eow"))
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_duemonth () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted)
    {
      Datetime due (get_date ("due"));
      if (due >= Datetime ("som") &&
          due <= Datetime ("eom"))
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_duequarter () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted)
    {
      Datetime due (get_date ("due"));
      if (due >= Datetime ("soq") &&
          due <= Datetime ("eoq"))
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_dueyear () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted)
    {
      Datetime due (get_date ("due"));
      if (due >= Datetime ("soy") &&
          due <= Datetime ("eoy"))
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_udaPresent () const
{
  for (auto& col : Context::getContext ().columns)
    if (col.second->is_uda () &&
        has (col.first))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_orphanPresent () const
{
  for (auto& att : data)
    if (att.first.compare (0, 11, "annotation_", 11) != 0)
      if (Context::getContext ().columns.find (att.first) == Context::getContext ().columns.end ())
        return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_overdue () const
{
  if (has ("due"))
  {
    Task::status status = getStatus ();

    if (status != Task::completed &&
        status != Task::deleted &&
        status != Task::recurring)
    {
      Task::dateState state = getDateState ("due");
      if (state == dateEarlierToday ||
          state == dateBeforeToday)
        return true;
    }
  }

  return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Attempt an FF4 parse first, using Task::parse, and in the event of an error
// try a JSON parse, otherwise a legacy parse (currently no legacy formats are
// supported).
//
// Note that FF1, FF2 and FF3 are no longer supported.
//
// start --> [ --> Att --> ] --> end
//              ^       |
//              +-------+
//
void Task::parse (const std::string& input)
{
  try
  {
    // File format version 4, from 2009-5-16 - now, v1.7.1+
    // This is the parse format tried first, because it is most used.
    data.clear ();

    if (input[0] == '[')
    {
      Pig pig (input);
      std::string line;
      if (pig.skip     ('[')       &&
          pig.getUntil (']', line) &&
          pig.skip     (']')       &&
          (pig.skip ('\n') || pig.eos ()))
      {
        if (line.length () == 0)
          throw std::string ("Empty record in input.");

        Pig attLine (line);
        std::string name;
        std::string value;
        while (!attLine.eos ())
        {
          if (attLine.getUntil (':', name) &&
              attLine.skip (':')           &&
              attLine.getQuoted ('"', value))
          {
#ifdef PRODUCT_TASKWARRIOR
            legacyAttributeMap (name);
#endif

            if (! name.compare (0, 11, "annotation_", 11))
              ++annotation_count;

            data[name] = decode (json::decode (value));
          }

          attLine.skip (' ');
        }

        std::string remainder;
        attLine.getRemainder (remainder);
        if (remainder.length ())
          throw std::string ("Unrecognized characters at end of line.");
      }
    }
    else if (input[0] == '{')
      parseJSON (input);
    else
      throw std::string ("Record not recognized as format 4.");
  }

  catch (const std::string&)
  {
    parseLegacy (input);
  }

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
// Note that all fields undergo encode/decode.
void Task::parseJSON (const std::string& line)
{
  // Parse the whole thing.
  json::value* root = json::parse (line);
  if (root &&
      root->type () == json::j_object)
    parseJSON ((json::object*) root);

  delete root;
}

////////////////////////////////////////////////////////////////////////////////
void Task::parseJSON (const json::object* root_obj)
{
  // For each object element...
  for (auto& i : root_obj->_data)
  {
    // If the attribute is a recognized column.
    std::string type = Task::attributes[i.first];
    if (!type.empty())
    {
      // Any specified id is ignored.
      if (i.first == "id")
        ;

      // Urgency, if present, is ignored.
      else if (i.first == "urgency")
        ;

      // TW-1274 Standardization.
      else if (i.first == "modification")
      {
        auto text = i.second->dump ();
        Lexer::dequote (text);
        Datetime d (text);
        set ("modified", d.toEpochString ());
      }

      // Dates are converted from ISO to epoch.
      else if (type == "date")
      {
        auto text = i.second->dump ();
        Lexer::dequote (text);
        Datetime d (text);
        set (i.first, text.empty() ? "" : d.toEpochString ());
      }

      // Tags are an array of JSON strings.
      else if (i.first == "tags" && i.second->type() == json::j_array)
      {
        auto* tags = (json::array*)i.second;
        for (auto& t : tags->_data)
        {
          auto* tag = (json::string*)t;
          addTag (tag->_data);
        }
      }
      // This is a temporary measure to accomodate a malformed JSON message from
      // Mirakel sync.
      //
      // 2016-02-21 Mirakel dropped sync support in late 2015. This can be
      //            removed in a later release.
      else if (i.first == "tags" && i.second->type() == json::j_string)
      {
        auto* tag = (json::string*)i.second;
        addTag (tag->_data);
      }

      // Dependencies can be exported as an array of strings.
      // 2016-02-21: This will be the only option in future releases.
      //             See other 2016-02-21 comments for details.
      else if (i.first == "depends" && i.second->type() == json::j_array)
      {
        auto* deps = (json::array*)i.second;
        for (auto& t : deps->_data)
        {
          auto* dep = (json::string*)t;
          addDependency (dep->_data);
        }
      }

      // Dependencies can be exported as a single comma-separated string.
      // 2016-02-21: Deprecated - see other 2016-02-21 comments for details.
      else if (i.first == "depends" && i.second->type() == json::j_string)
      {
        auto* deps = (json::string*)i.second;
        auto uuids = split (deps->_data, ',');

        for (const auto& uuid : uuids)
          addDependency (uuid);
      }

      // Strings are decoded.
      else if (type == "string")
      {
        auto text = i.second->dump ();
        Lexer::dequote (text);
        set (i.first, json::decode (text));
      }

      // Other types are simply added.
      else
      {
        auto text = i.second->dump ();
        Lexer::dequote (text);
        set (i.first, text);
      }
    }

    // UDA orphans and annotations do not have columns.
    else
    {
      // Annotations are an array of JSON objects with 'entry' and
      // 'description' values and must be converted.
      if (i.first == "annotations")
      {
        std::map <std::string, std::string> annos;

        auto* atts = (json::array*)i.second;
        for (auto& annotations : atts->_data)
        {
          auto* annotation = (json::object*)annotations;
          json::string* when = (json::string*)annotation->_data["entry"];
          json::string* what = (json::string*)annotation->_data["description"];

          if (! when)
            throw format ("Annotation is missing an entry date: {1}", root_obj-> dump ());

          if (! what)
            throw format ("Annotation is missing a description: {1}", root_obj->dump ());

          std::string name = "annotation_" + Datetime (when->_data).toEpochString ();
          annos.insert (std::make_pair (name, json::decode (what->_data)));
        }

        setAnnotations (annos);
      }

      // UDA Orphan - must be preserved.
      else
      {
#ifdef PRODUCT_TASKWARRIOR
        std::stringstream message;
        message << "Task::parseJSON found orphan '"
                << i.first
                << "' with value '"
                << i.second
                << "' --> preserved\n";
        Context::getContext ().debug (message.str ());
#endif
        auto text = i.second->dump ();
        Lexer::dequote (text);
        set (i.first, json::decode (text));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// No legacy formats are currently supported as of 2.4.0.
void Task::parseLegacy (const std::string& line)
{
  switch (determineVersion (line))
  {
  // File format version 1, from 2006-11-27 - 2007-12-31, v0.x+ - v0.9.3
  case 1: throw std::string ("Taskwarrior no longer supports file format 1, originally used between 27 November 2006 and 31 December 2007.");

  // File format version 2, from 2008-1-1 - 2009-3-23, v0.9.3 - v1.5.0
  case 2: throw std::string ("Taskwarrior no longer supports file format 2, originally used between 1 January 2008 and 12 April 2009.");

  // File format version 3, from 2009-3-23 - 2009-05-16, v1.6.0 - v1.7.1
  case 3: throw std::string ("Taskwarrior no longer supports file format 3, originally used between 23 March 2009 and 16 May 2009.");

  // File format version 4, from 2009-05-16 - today, v1.7.1+
  case 4:
    break;

  default:
#ifdef PRODUCT_TASKWARRIOR
    std::stringstream message;
    message << "Invalid fileformat at line '"
            << line
            << '\'';
    Context::getContext ().debug (message.str ());
#endif
    throw std::string ("Unrecognized Taskwarrior file format or blank line in data.");
    break;
  }

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
// The format is:
//
//   [ <name>:<value> ... ]
//
std::string Task::composeF4 () const
{
  std::string ff4 = "[";

  bool first = true;
  for (const auto& it : data)
  {
    // Orphans have no type, treat as string.
    std::string type = Task::attributes[it.first];
    if (type.empty())
      type = "string";

    // If there is a value.
    if (!it.second.empty())
    {
      ff4 += (first ? "" : " ");
      ff4 += it.first;
      ff4 += ":\"";
      if (type == "string")
        ff4 += encode (json::encode (it.second));
      else
        ff4 += it.second;
      ff4 += '"';

      first = false;
    }
  }

  ff4 += ']';
  return ff4;
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::composeJSON (bool decorate /*= false*/) const
{
  std::stringstream out;
  out << '{';

  // ID inclusion is optional, but not a good idea, because it remains correct
  // only until the next gc.
  if (decorate)
    out << "\"id\":" << id << ',';

  // First the non-annotations.
  int attributes_written = 0;
  for (auto& i : data)
  {
    // Annotations are not written out here.
    if (! i.first.compare (0, 11, "annotation_", 11))
      continue;

    // If value is an empty string, do not ever output it
    if (i.second.empty())
        continue;

    if (attributes_written)
      out << ',';

    std::string type = Task::attributes[i.first];
    if (type.empty())
      type = "string";

    // Date fields are written as ISO 8601.
    if (type == "date")
    {
      Datetime d (i.second);
      out << '"'
          << (i.first == "modification" ? "modified" : i.first)
          << "\":\""
          // Date was deleted, do not export parsed empty string
          << (i.second.empty() ? "" : d.toISO ())
          << '"';

      ++attributes_written;
    }

/*
    else if (type == "duration")
    {
      // TODO Emit Datetime
    }
*/
    else if (type == "numeric")
    {
      out << '"'
          << i.first
          << "\":"
          << i.second;

      ++attributes_written;
    }

    // Tags are converted to an array.
    else if (i.first == "tags")
    {
      auto tags = split (i.second, ',');

      out << "\"tags\":[";

      int count = 0;
      for (const auto& i : tags)
      {
        if (count++)
          out << ',';

        out << '"' << i << '"';
      }

      out << ']';
      ++attributes_written;
    }

    // Dependencies are an array by default.
    else if (i.first == "depends"
#ifdef PRODUCT_TASKWARRIOR
    // 2016-02-20: Taskwarrior 2.5.0 introduced the 'json.depends.array' setting
    //             which defaulted to 'on', and emitted this JSON for
    //             dependencies:
    //
    //             With json.depends.array=on    "depends":["<uuid>","<uuid>"]
    //             With json.depends.array=off   "depends":"<uuid>,<uuid>"
    //
    //             Taskwarrior 2.5.1 defaults this to 'off', because Taskserver
    //             1.0.0 and 1.1.0 both expect that. Taskserver 1.2.0 will
    //             accept both forms, but emit the 'off' variant.
    //
    //             When Taskwarrior 2.5.0 is no longer the dominant version,
    //             and Taskserver 1.2.0 is released, the default for
    //             'json.depends.array' can revert to 'on'.

             && Context::getContext ().config.getBoolean ("json.depends.array")
#endif
            )
    {
      auto deps = split (i.second, ',');

      out << "\"depends\":[";

      int count = 0;
      for (const auto& i : deps)
      {
        if (count++)
          out << ',';

        out << '"' << i << '"';
      }

      out << ']';
      ++attributes_written;
    }

    // Everything else is a quoted value.
    else
    {
      out << '"'
          << i.first
          << "\":\""
          << (type == "string" ? json::encode (i.second) : i.second)
          << '"';

      ++attributes_written;
    }
  }

  // Now the annotations, if any.
  if (annotation_count)
  {
    out << ','
        << "\"annotations\":[";

    int annotations_written = 0;
    for (auto& i : data)
    {
      if (! i.first.compare (0, 11, "annotation_", 11))
      {
        if (annotations_written)
          out << ',';

        Datetime d (i.first.substr (11));
        out << "{\"entry\":\""
            << d.toISO ()
            << "\",\"description\":\""
            << json::encode (i.second)
            << "\"}";

        ++annotations_written;
      }
    }

    out << ']';
  }

#ifdef PRODUCT_TASKWARRIOR
  // Include urgency.
  if (decorate)
    out << ','
        << "\"urgency\":"
        << urgency_c ();
#endif

  out << '}';
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
int Task::getAnnotationCount () const
{
  int count = 0;
  for (auto& ann : data)
    if (! ann.first.compare (0, 11, "annotation_", 11))
      ++count;

  return count;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::hasAnnotations () const
{
  return annotation_count ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// The timestamp is part of the name:
//    annotation_1234567890:"..."
//
// Note that the time is incremented (one second) in order to find a unique
// timestamp.
void Task::addAnnotation (const std::string& description)
{
  time_t now = time (nullptr);
  std::string key;

  do
  {
    key = "annotation_" + format ((int) now);
    ++now;
  }
  while (has (key));

  data[key] = json::decode (description);
  ++annotation_count;
  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeAnnotations ()
{
  // Erase old annotations.
  auto i = data.begin ();
  while (i != data.end ())
  {
    if (! i->first.compare (0, 11, "annotation_", 11))
    {
      --annotation_count;
      data.erase (i++);
    }
    else
      i++;
  }

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
std::map <std::string, std::string> Task::getAnnotations () const
{
  std::map <std::string, std::string> a;
  for (auto& ann : data)
    if (! ann.first.compare (0, 11, "annotation_", 11))
      a.insert (ann);

  return a;
}

////////////////////////////////////////////////////////////////////////////////
void Task::setAnnotations (const std::map <std::string, std::string>& annotations)
{
  // Erase old annotations.
  removeAnnotations ();

  for (auto& anno : annotations)
    data.insert (anno);

  annotation_count = annotations.size ();
  recalc_urgency = true;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
void Task::addDependency (int depid)
{
  // Check that id is resolvable.
  std::string uuid = Context::getContext ().tdb2.pending.uuid (depid);
  if (uuid.empty())
    throw format ("Could not create a dependency on task {1} - not found.", depid);

  std::string depends = get ("depends");
  if (depends.find (uuid) != std::string::npos)
  {
    Context::getContext ().footnote (format ("Task {1} already depends on task {2}.", id, depid));
    return;
  }

  addDependency(uuid);
}
#endif

////////////////////////////////////////////////////////////////////////////////
void Task::addDependency (const std::string& uuid)
{
  if (uuid == get ("uuid"))
    throw std::string ("A task cannot be dependent on itself.");

  // Store the dependency.
  std::string depends = get ("depends");
  if (!depends.empty())
  {
    // Check for extant dependency.
    if (depends.find (uuid) == std::string::npos)
      set ("depends", depends + ',' + uuid);
    else
    {
#ifdef PRODUCT_TASKWARRIOR
      Context::getContext ().footnote (format ("Task {1} already depends on task {2}.", get ("uuid"), uuid));
#endif
      return;
    }
  }
  else
    set ("depends", uuid);

  // Prevent circular dependencies.
#ifdef PRODUCT_TASKWARRIOR
  if (dependencyIsCircular (*this))
    throw std::string ("Circular dependency detected and disallowed.");
#endif

  recalc_urgency = true;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
void Task::removeDependency (const std::string& uuid)
{
  auto deps = split (get ("depends"), ',');

  auto i = std::find (deps.begin (), deps.end (), uuid);
  if (i != deps.end ())
  {
    deps.erase (i);
    set ("depends", join (",", deps));
    recalc_urgency = true;
  }
  else
    throw format ("Could not delete a dependency on task {1} - not found.", uuid);
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeDependency (int id)
{
  std::string depends = get ("depends");
  std::string uuid = Context::getContext ().tdb2.pending.uuid (id);
  if (!uuid.empty() && depends.find (uuid) != std::string::npos)
    removeDependency (uuid);
  else
    throw format ("Could not delete a dependency on task {1} - not found.", id);
}

////////////////////////////////////////////////////////////////////////////////
std::vector <int> Task::getDependencyIDs () const
{
  std::vector <int> all;
  for (auto& dep : split (get ("depends"), ','))
    all.push_back (Context::getContext ().tdb2.pending.id (dep));

  return all;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Task::getDependencyUUIDs () const
{
  return split (get ("depends"), ',');
}

////////////////////////////////////////////////////////////////////////////////
std::vector <Task> Task::getDependencyTasks () const
{
  std::vector <Task> all;
  for (auto& dep : split (get ("depends"), ','))
  {
    Task task;
    Context::getContext ().tdb2.get (dep, task);
    all.push_back (task);
  }

  return all;
}
#endif

////////////////////////////////////////////////////////////////////////////////
int Task::getTagCount () const
{
  auto tags = split (get ("tags"), ',');
  return (int) tags.size ();
}

////////////////////////////////////////////////////////////////////////////////
//
//              OVERDUE YESTERDAY DUE TODAY TOMORROW WEEK MONTH YEAR
// due:-1week      Y       -       -    -       -      ?    ?     ?
// due:-1day       Y       Y       -    -       -      ?    ?     ?
// due:today       Y       -       Y    Y       -      ?    ?     ?
// due:tomorrow    -       -       Y    -       Y      ?    ?     ?
// due:3days       -       -       Y    -       -      ?    ?     ?
// due:1month      -       -       -    -       -      -    -     ?
// due:1year       -       -       -    -       -      -    -     -
//
bool Task::hasTag (const std::string& tag) const
{
  // Synthetic tags - dynamically generated, but do not occupy storage space.
  // Note: This list must match that in CmdInfo::execute.
  // Note: This list must match that in ::feedback_reserved_tags.
  if (isupper (tag[0]))
  {
    if (tag == "BLOCKED")   return is_blocked;
    if (tag == "UNBLOCKED") return !is_blocked;
    if (tag == "BLOCKING")  return is_blocking;
#ifdef PRODUCT_TASKWARRIOR
    if (tag == "READY")     return is_ready ();
    if (tag == "DUE")       return is_due ();
    if (tag == "DUETODAY")  return is_duetoday ();                     // 2016-03-29: Deprecated in 2.6.0
    if (tag == "TODAY")     return is_duetoday ();
    if (tag == "YESTERDAY") return is_dueyesterday ();
    if (tag == "TOMORROW")  return is_duetomorrow ();
    if (tag == "OVERDUE")   return is_overdue ();
    if (tag == "WEEK")      return is_dueweek ();
    if (tag == "MONTH")     return is_duemonth ();
    if (tag == "QUARTER")   return is_duequarter ();
    if (tag == "YEAR")      return is_dueyear ();
#endif
    if (tag == "ACTIVE")    return has ("start");
    if (tag == "SCHEDULED") return has ("scheduled");
    if (tag == "CHILD")     return has ("parent") || has ("template"); // 2017-01-07: Deprecated in 2.6.0
    if (tag == "INSTANCE")  return has ("template") || has ("parent");
    if (tag == "UNTIL")     return has ("until");
    if (tag == "ANNOTATED") return hasAnnotations ();
    if (tag == "TAGGED")    return has ("tags");
    if (tag == "PARENT")    return has ("mask") || has ("last");       // 2017-01-07: Deprecated in 2.6.0
    if (tag == "TEMPLATE")  return has ("last") || has ("mask");
    if (tag == "WAITING")   return get ("status") == "waiting";
    if (tag == "PENDING")   return get ("status") == "pending";
    if (tag == "COMPLETED") return get ("status") == "completed";
    if (tag == "DELETED")   return get ("status") == "deleted";
#ifdef PRODUCT_TASKWARRIOR
    if (tag == "UDA")       return is_udaPresent ();
    if (tag == "ORPHAN")    return is_orphanPresent ();
    if (tag == "LATEST")    return id == Context::getContext ().tdb2.latest_id ();
#endif
    if (tag == "PROJECT")   return has ("project");
    if (tag == "PRIORITY")  return has ("priority");
  }

  // Concrete tags.
  auto tags = split (get ("tags"), ',');

  if (std::find (tags.begin (), tags.end (), tag) != tags.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Task::addTag (const std::string& tag)
{
  auto tags = split (get ("tags"), ',');

  if (std::find (tags.begin (), tags.end (), tag) == tags.end ())
  {
    tags.push_back (tag);
    set ("tags", join (",", tags));

    recalc_urgency = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::addTags (const std::vector <std::string>& tags)
{
  remove ("tags");

  for (auto& tag : tags)
    addTag (tag);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Task::getTags () const
{
  return split (get ("tags"), ',');
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeTag (const std::string& tag)
{
  auto tags = split (get ("tags"), ',');

  auto i = std::find (tags.begin (), tags.end (), tag);
  if (i != tags.end ())
  {
    tags.erase (i);
    set ("tags", join (",", tags));
  }

  recalc_urgency = true;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
// A UDA Orphan is an attribute that is not represented in context.columns.
std::vector <std::string> Task::getUDAOrphanUUIDs () const
{
  std::vector <std::string> orphans;
  for (auto& it : data)
    if (it.first.compare (0, 11, "annotation_", 11) != 0)
      if (Context::getContext ().columns.find (it.first) == Context::getContext ().columns.end ())
        orphans.push_back (it.first);

  return orphans;
}

////////////////////////////////////////////////////////////////////////////////
void Task::substitute (
  const std::string& from,
  const std::string& to,
  const std::string& flags)
{
  bool global = (flags.find ('g') != std::string::npos ? true : false);

  // Get the data to modify.
  std::string description = get ("description");
  auto annotations = getAnnotations ();

  // Count the changes, so we know whether to proceed to annotations, after
  // modifying description.
  int changes = 0;
  bool done = false;

  // Regex support is optional.
  if (Task::regex)
  {
    // Create the regex.
    RX rx (from, Task::searchCaseSensitive);
    std::vector <int> start;
    std::vector <int> end;

    // Perform all subs on description.
    if (rx.match (start, end, description))
    {
      int skew = 0;
      for (unsigned int i = 0; i < start.size () && !done; ++i)
      {
        description.replace (start[i] + skew, end[i] - start[i], to);
        skew += to.length () - (end[i] - start[i]);
        ++changes;

        if (!global)
          done = true;
      }
    }

    if (!done)
    {
      // Perform all subs on annotations.
      for (auto& it : annotations)
      {
        start.clear ();
        end.clear ();
        if (rx.match (start, end, it.second))
        {
          int skew = 0;
          for (unsigned int i = 0; i < start.size () && !done; ++i)
          {
            it.second.replace (start[i + skew], end[i] - start[i], to);
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

    while ((pos = ::find (description, from, pos, Task::searchCaseSensitive)) != std::string::npos && !done)
    {
      description.replace (pos + skew, from.length (), to);
      skew += to.length () - from.length ();

      pos += to.length ();
      ++changes;

      if (!global)
        done = true;

      if (++counter > APPROACHING_INFINITY)
        throw format ("Terminated substitution because more than {1} changes were made - infinite loop protection.", APPROACHING_INFINITY);
    }

    if (!done)
    {
      // Perform all subs on annotations.
      counter = 0;
      for (auto& anno : annotations)
      {
        pos = 0;
        skew = 0;
        while ((pos = ::find (anno.second, from, pos, Task::searchCaseSensitive)) != std::string::npos && !done)
        {
          anno.second.replace (pos + skew, from.length (), to);
          skew += to.length () - from.length ();

          pos += to.length ();
          ++changes;

          if (!global)
            done = true;

          if (++counter > APPROACHING_INFINITY)
            throw format ("Terminated substitution because more than {1} changes were made - infinite loop protection.", APPROACHING_INFINITY);
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
#endif

////////////////////////////////////////////////////////////////////////////////
// The purpose of Task::validate is three-fold:
//   1) To provide missing attributes where possible
//   2) To provide suitable warnings about odd states
//   3) To generate errors when the inconsistencies are not fixable
//
void Task::validate (bool applyDefault /* = true */)
{
  Task::status status = Task::pending;
  if (!get ("status").empty())
    status = getStatus ();

  // 1) Provide missing attributes where possible
  // Provide a UUID if necessary. Validate if present.
  std::string uid = get ("uuid");
  if (has ("uuid") && !uid.empty())
  {
    Lexer lex (uid);
    std::string token;
    Lexer::Type type;
    if (! lex.isUUID (token, type, true))
      throw format ("Not a valid UUID '{1}'.", uid);
  }
  else
    set ("uuid", uuid ());

  // TODO Obsolete remove for 2.6.0
  // Recurring tasks get a special status.
  if (status == Task::pending                     &&
      has ("due")                                 &&
      has ("recur")                               &&
      (! has ("parent") || get ("parent").empty())  &&
      (! has ("template") || get ("template").empty()))
  {
    status = Task::recurring;
  }
/*
  // TODO Add for 2.6.0
  if (status == Task::pending &&
      has ("due")             &&
      has ("recur")          &&
      (! has ("template") || get ("template") == ""))
  {
    status = Task::recurring;
  }
*/

  // Tasks with a wait: date get a special status.
  else if (status == Task::pending &&
           has ("wait")            &&
           !get ("wait").empty())
    status = Task::waiting;

  // By default, tasks are pending.
  else if (! has ("status") || get ("status").empty())
    status = Task::pending;

  // Default to 'periodic' type recurrence.
  if (status == Task::recurring &&
      (! has ("rtype") || get ("rtype").empty()))
  {
    set ("rtype", "periodic");
  }

  // Store the derived status.
  setStatus (status);

#ifdef PRODUCT_TASKWARRIOR
  // Provide an entry date unless user already specified one.
  if (! has ("entry") || get ("entry").empty())
    setAsNow ("entry");

  // Completed tasks need an end date, so inherit the entry date.
  if ((status == Task::completed || status == Task::deleted) &&
      (! has ("end") || get ("end").empty()))
    setAsNow ("end");

  // Pending tasks cannot have an end date, remove if present
  if ((status == Task::pending) && (!get ("end").empty()))
    remove ("end");

  // Provide an entry date unless user already specified one.
  if (! has ("modified") || get ("modified").empty())
    setAsNow ("modified");

  if (applyDefault && (! has ("parent") || get ("parent").empty()))
  {
    // Override with default.project, if not specified.
    if (!Task::defaultProject.empty() &&
        ! has ("project"))
    {
      if (Context::getContext ().columns["project"]->validate (Task::defaultProject))
        set ("project", Task::defaultProject);
    }

    // Override with default.due, if not specified.
    if (!Task::defaultDue.empty() &&
        ! has ("due"))
    {
      if (Context::getContext ().columns["due"]->validate (Task::defaultDue))
      {
        Duration dur (Task::defaultDue);
        if (dur.toTime_t () != 0)
          set ("due", (Datetime () + dur.toTime_t ()).toEpoch ());
        else
          set ("due", Datetime (Task::defaultDue).toEpoch ());
      }
    }

    // Override with default.scheduled, if not specified.
    if (!Task::defaultScheduled.empty() &&
        ! has ("scheduled"))
    {
      if (Context::getContext ().columns["scheduled"]->validate (Task::defaultScheduled))
      {
        Duration dur (Task::defaultScheduled);
        if (dur.toTime_t () != 0)
          set ("scheduled", (Datetime () + dur.toTime_t ()).toEpoch ());
        else
          set ("scheduled", Datetime (Task::defaultScheduled).toEpoch ());
      }
    }

    // If a UDA has a default value in the configuration,
    // override with uda.(uda).default, if not specified.
    // Gather a list of all UDAs with a .default value
    std::vector <std::string> udas;
    for (auto& var : Context::getContext ().config)
    {
      if (! var.first.compare (0, 4, "uda.", 4) &&
          var.first.find (".default") != std::string::npos)
      {
        auto period = var.first.find ('.', 4);
        if (period != std::string::npos)
          udas.push_back (var.first.substr (4, period - 4));
      }
    }

    if (!udas.empty())
    {
      // For each of those, setup the default value on the task now,
      // of course only if we don't have one on the command line already
      for (auto& uda : udas)
      {
        std::string defVal= Context::getContext ().config.get ("uda." + uda + ".default");

        // If the default is empty, or we already have a value, skip it
        if (!defVal.empty() && get (uda).empty())
          set (uda, defVal);
      }
    }
  }
#endif

  // 2) To provide suitable warnings about odd states

  // Date relationships.
  validate_before ("wait",      "due");
  validate_before ("entry",     "start");
  validate_before ("entry",     "end");
  validate_before ("wait",      "scheduled");
  validate_before ("scheduled", "start");
  validate_before ("scheduled", "due");
  validate_before ("scheduled", "end");

  // 3) To generate errors when the inconsistencies are not fixable

  // There is no fixing a missing description.
  if (! has ("description"))
    throw std::string ("A task must have a description.");
  else if (get ("description").empty())
    throw std::string ("Cannot add a task that is blank.");

  // Cannot have a recur frequency with no due date - when would it recur?
  if (has ("recur") && (! has ("due") || get ("due").empty()))
    throw std::string ("A recurring task must also have a 'due' date.");

  // Recur durations must be valid.
  if (has ("recur"))
  {
    std::string value = get ("recur");
    if (!value.empty())
    {
      Duration p;
      std::string::size_type i = 0;
      if (! p.parse (value, i))
        // TODO Ideal location to map unsupported old recurrence periods to supported values.
        throw format ("The recurrence value '{1}' is not valid.", value);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::validate_before (const std::string& left, const std::string& right)
{
#ifdef PRODUCT_TASKWARRIOR
  if (has (left) &&
      has (right))
  {
    Datetime date_left (get_date (left));
    Datetime date_right (get_date (right));

    // if date is zero, then it is being removed (e.g. "due: wait:1day")
    if (date_left > date_right && date_right.toEpoch () != 0)
      Context::getContext ().footnote (format ("Warning: You have specified that the '{1}' date is after the '{2}' date.", left, right));
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Encode values prior to serialization.
//   [  -> &open;
//   ]  -> &close;
std::string Task::encode (const std::string& value) const
{
  auto modified = str_replace (value,    "[", "&open;");
  return          str_replace (modified, "]", "&close;");
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   [  <- &open;
//   ]  <- &close;
std::string Task::decode (const std::string& value) const
{
  if (value.find ('&') == std::string::npos)
    return value;

  auto modified = str_replace (value,    "&open;",  "[");
  return          str_replace (modified, "&close;", "]");
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
    auto tagAtts  = line.find ("] [", 0);
    auto attsAnno = line.find ("] [", tagAtts + 1);
    auto annoDesc = line.find ("] ",  attsAnno + 1);
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
  else if (line.find ("X [") == 0                      ||
           (line[0] == '['                             &&
            line.substr (line.length () - 1, 1) != "]" &&
            line.length () > 3))
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
#ifdef PRODUCT_TASKWARRIOR
  value += fabsf (Task::urgencyProjectCoefficient)     > epsilon ? (urgency_project ()     * Task::urgencyProjectCoefficient)     : 0.0;
  value += fabsf (Task::urgencyActiveCoefficient)      > epsilon ? (urgency_active ()      * Task::urgencyActiveCoefficient)      : 0.0;
  value += fabsf (Task::urgencyScheduledCoefficient)   > epsilon ? (urgency_scheduled ()   * Task::urgencyScheduledCoefficient)   : 0.0;
  value += fabsf (Task::urgencyWaitingCoefficient)     > epsilon ? (urgency_waiting ()     * Task::urgencyWaitingCoefficient)     : 0.0;
  value += fabsf (Task::urgencyBlockedCoefficient)     > epsilon ? (urgency_blocked ()     * Task::urgencyBlockedCoefficient)     : 0.0;
  value += fabsf (Task::urgencyAnnotationsCoefficient) > epsilon ? (urgency_annotations () * Task::urgencyAnnotationsCoefficient) : 0.0;
  value += fabsf (Task::urgencyTagsCoefficient)        > epsilon ? (urgency_tags ()        * Task::urgencyTagsCoefficient)        : 0.0;
  value += fabsf (Task::urgencyDueCoefficient)         > epsilon ? (urgency_due ()         * Task::urgencyDueCoefficient)         : 0.0;
  value += fabsf (Task::urgencyBlockingCoefficient)    > epsilon ? (urgency_blocking ()    * Task::urgencyBlockingCoefficient)    : 0.0;
  value += fabsf (Task::urgencyAgeCoefficient)         > epsilon ? (urgency_age ()         * Task::urgencyAgeCoefficient)         : 0.0;

  // Tag- and project-specific coefficients.
  for (auto& var : Task::coefficients)
  {
    if (fabs (var.second) > epsilon)
    {
      if (! var.first.compare (0, 13, "urgency.user.", 13))
      {
        // urgency.user.project.<project>.coefficient
        auto end = std::string::npos;
        if (var.first.substr (13, 8) == "project." &&
            (end = var.first.find (".coefficient")) != std::string::npos)
        {
          std::string project = var.first.substr (21, end - 21);

          if (get ("project").find (project) == 0)
            value += var.second;
        }

        // urgency.user.tag.<tag>.coefficient
        if (var.first.substr (13, 4) == "tag." &&
            (end = var.first.find (".coefficient")) != std::string::npos)
        {
          std::string tag = var.first.substr (17, end - 17);

          if (hasTag (tag))
            value += var.second;
        }

        // urgency.user.keyword.<keyword>.coefficient
        if (var.first.substr (13, 8) == "keyword." &&
            (end = var.first.find (".coefficient")) != std::string::npos)
        {
          std::string keyword = var.first.substr (21, end - 21);

          if (get ("description").find (keyword) != std::string::npos)
            value += var.second;
        }
      }
      else if (var.first.substr (0, 12) == "urgency.uda.")
      {
        // urgency.uda.<name>.coefficient
        // urgency.uda.<name>.<value>.coefficient
        auto end = var.first.find (".coefficient");
        if (end != std::string::npos)
        {
          const std::string uda = var.first.substr (12, end - 12);
          auto dot = uda.find ('.');
          if (dot == std::string::npos)
          {
            // urgency.uda.<name>.coefficient
            if (has (uda))
              value += var.second;
          }
          else
          {
            // urgency.uda.<name>.<value>.coefficient
            if (get (uda.substr(0, dot)) == uda.substr(dot+1))
              value += var.second;
          }
        }
      }
    }
  }

  if (is_blocking && Context::getContext ().config.getBoolean ("urgency.inherit"))
  {
    float prev = value;
    value = std::max (value, urgency_inherit ());

    // This is a hackish way of making sure parent tasks are sorted above
    // child tasks.  For reports that hide blocked tasks, this is not needed.
    if (prev < value)
      value += 0.01;
  }
#endif

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
float Task::urgency_inherit () const
{
  float v = FLT_MIN;
#ifdef PRODUCT_TASKWARRIOR
  // Calling dependencyGetBlocked is rather expensive.
  // It is called recursively for each dependency in the chain here.
  for (auto& task : dependencyGetBlocked (*this))
  {
    // Find highest urgency in all blocked tasks.
    v = std::max (v, task.urgency ());
  }
#endif

  return v;
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
float Task::urgency_scheduled () const
{
  if (has ("scheduled") &&
      get_date ("scheduled") < time (nullptr))
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_waiting () const
{
  if (get_ref ("status") == "waiting")
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
// A task is blocked only if the task it depends upon is pending/waiting.
float Task::urgency_blocked () const
{
  if (is_blocked)
    return 1.0;

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_annotations () const
{
       if (annotation_count >= 3) return 1.0;
  else if (annotation_count == 2) return 0.9;
  else if (annotation_count == 1) return 0.8;

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
//
//     Past                  Present                              Future
//     Overdue               Due                                     Due
//
//     -7 -6 -5 -4 -3 -2 -1  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 days
//
// <-- 1.0                         linear                            0.2 -->
//     capped                                                        capped
//
//
float Task::urgency_due () const
{
  if (has ("due"))
  {
    Datetime now;
    Datetime due (get_date ("due"));

    // Map a range of 21 days to the value 0.2 - 1.0
    float days_overdue = (now - due) / 86400.0;
         if (days_overdue >= 7.0)   return 1.0;   // < 1 wk ago
    else if (days_overdue >= -14.0) return ((days_overdue + 14.0) * 0.8 / 21.0) + 0.2;
    else                            return 0.2;   // > 2 wks
  }

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_age () const
{
  assert (has ("entry"));

  Datetime now;
  Datetime entry (get_date ("entry"));
  int age = (now - entry) / 86400;  // in days

  if (Task::urgencyAgeMax == 0 || age > Task::urgencyAgeMax)
    return 1.0;

  return (1.0 * age / Task::urgencyAgeMax);
}

////////////////////////////////////////////////////////////////////////////////
float Task::urgency_blocking () const
{
  if (is_blocking)
    return 1.0;

  return 0.0;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
// Arguably does not belong here. This method reads the parse tree and calls
// Task methods. It could be a standalone function with no loss in access, as
// well as reducing the object depdendencies of Task.
//
// It came from the Command base object, but doesn't really belong there either.
void Task::modify (modType type, bool text_required /* = false */)
{
  std::string label = "  [1;37;43mMODIFICATION[0m ";

  // Need this for later comparison.
  auto originalStatus = getStatus ();

  std::string text;
  bool mods = false;
  for (auto& a : Context::getContext ().cli2._args)
  {
    if (a.hasTag ("MODIFICATION"))
    {
      if (a._lextype == Lexer::Type::pair)
      {
        // 'canonical' is the canonical name. Needs to be said.
        // 'value' requires eval.
        std::string name  = a.attribute ("canonical");
        std::string value = a.attribute ("value");
        if (value.empty()     ||
            value == "''"   ||
            value == "\"\"")
        {
          // ::composeF4 will skip if the value is blank, but the presence of
          // the attribute will prevent ::validate from applying defaults.
          if ((has (name) && !get (name).empty()) ||
              (name == "due"       && Context::getContext ().config.has ("default.due")) ||
              (name == "scheduled" && Context::getContext ().config.has ("default.scheduled")) ||
              (name == "project"   && Context::getContext ().config.has ("default.project")))
          {
            mods = true;
            set (name, "");
          }

          Context::getContext ().debug (label + name + " <-- ''");
        }
        else
        {
          Lexer::dequote (value);

          // Get the column info. Some columns are not modifiable.
          Column* column = Context::getContext ().columns[name];
          if (! column ||
              ! column->modifiable ())
            throw format ("The '{1}' attribute does not allow a value of '{2}'.", name, value);

          // Delegate modification to the column object or their base classes.
          if (name == "depends"             ||
              name == "tags"                ||
              name == "recur"               ||
              column->type () == "date"     ||
              column->type () == "duration" ||
              column->type () == "numeric"  ||
              column->type () == "string")
          {
            column->modify (*this, value);
            mods = true;
          }

          else
            throw format ("Unrecognized column type '{1}' for column '{2}'", column->type (), name);
        }
      }

      // Perform description/annotation substitution.
      else if (a._lextype == Lexer::Type::substitution)
      {
        Context::getContext ().debug (label + "substitute " + a.attribute ("raw"));
        substitute (a.attribute ("from"),
                    a.attribute ("to"),
                    a.attribute ("flags"));
        mods = true;
      }

      // Tags need special handling because they are essentially a vector stored
      // in a single string, therefore Task::{add,remove}Tag must be called as
      // appropriate.
      else if (a._lextype == Lexer::Type::tag)
      {
        std::string tag = a.attribute ("name");
        feedback_reserved_tags (tag);

        if (a.attribute ("sign") == "+")
        {
          Context::getContext ().debug (label + "tags <-- add '" + tag + '\'');
          addTag (tag);
          feedback_special_tags (*this, tag);
        }
        else
        {
          Context::getContext ().debug (label + "tags <-- remove '" + tag + '\'');
          removeTag (tag);
        }

        mods = true;
      }

      // Unknown args are accumulated as though they were WORDs.
      else
      {
        if (!text.empty())
          text += ' ';
        text += a.attribute ("raw");
      }
    }
  }

  // Task::modType determines what happens to the WORD arguments, if there are
  //  any.
  if (!text.empty())
  {
    Lexer::dequote (text);

    switch (type)
    {
    case modReplace:
      Context::getContext ().debug (label + "description <-- '" + text + '\'');
      set ("description", text);
      break;

    case modPrepend:
      Context::getContext ().debug (label + "description <-- '" + text + "' + description");
      set ("description", text + ' ' + get ("description"));
      break;

    case modAppend:
      Context::getContext ().debug (label + "description <-- description + '" + text + '\'');
      set ("description", get ("description") + ' ' + text);
      break;

    case modAnnotate:
      Context::getContext ().debug (label + "new annotation <-- '" + text + '\'');
      addAnnotation (text);
      break;
    }
  }
  else if (! mods && text_required)
    throw std::string ("Additional text must be provided.");

  // Modifying completed/deleted tasks generates a message, if the modification
  // does not change status.
  if ((getStatus () == Task::completed || getStatus () == Task::deleted) &&
      getStatus () == originalStatus)
  {
    auto uuid = get ("uuid").substr (0, 8);
    Context::getContext ().footnote (format ("Note: Modified task {1} is {2}.  You may wish to make this task pending with: task {3} modify status:pending", uuid, get ("status"), uuid));
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
