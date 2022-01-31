////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
#include <stdlib.h>
#include <assert.h>
#include <string>
#ifdef PRODUCT_TASKWARRIOR
#include <math.h>
#include <ctype.h>
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

static const float epsilon = 0.000001;
#endif

std::string Task::defaultProject   = "";
std::string Task::defaultDue       = "";
std::string Task::defaultScheduled = "";
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

static const std::string dummy ("");

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
bool Task::operator!= (const Task& other)
{
  return !(*this == other);
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
  // for compatibility, parse `w` as pending; Task::getStatus will
  // apply the virtual waiting status if appropriate
  else if (input[0] == 'w') return Task::pending;

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
const std::string Task::identifier (bool shortened /* = false */) const
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
std::vector <std::string> Task::all () const
{
  std::vector <std::string> all;
  for (const auto& i : data)
    all.push_back (i.first);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Task::get (const std::string& name) const
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
  data[name] = value;

  if (isAnnotationAttr (name))
    ++annotation_count;

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::set (const std::string& name, long long value)
{
  data[name] = format (value);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::remove (const std::string& name)
{
  if (data.erase (name))
    recalc_urgency = true;

  if (isAnnotationAttr (name))
    --annotation_count;
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::getStatus () const
{
  if (! has ("status"))
    return Task::pending;

  auto status = textToStatus (get ("status"));

  // Implement the "virtual" Task::waiting status, which is not stored on-disk
  // but is defined as a pending task with a `wait` attribute in the future.
  // This is workaround for 2.6.0, remove in 3.0.0.
  if (status == Task::pending && is_waiting ()) {
      return Task::waiting;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
void Task::setStatus (Task::status status)
{
  // the 'waiting' status is a virtual version of 'pending', so translate
  // that back to 'pending' here
  if (status == Task::waiting)
      status = Task::pending;

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
// An empty task is typically a "dummy", such as in DOM evaluation, which may or
// may not occur in the context of a task.
bool Task::is_empty () const
{
  return data.size () == 0;
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
    if (! isAnnotationAttr (att.first) &&
        ! isTagAttr (att.first) &&
        ! isDepAttr (att.first) &&
        Context::getContext ().columns.find (att.first) == Context::getContext ().columns.end ())
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
// Task is considered waiting if it's pending and the wait attribute is set as
// future datetime value.
// While this is not consistent with other attribute-based virtual tags, such
// as +BLOCKED, it is more backwards compatible with how +WAITING virtual tag
// behaved in the past, when waiting had a dedicated status value.
bool Task::is_waiting () const
{
  if (has ("wait") && get ("status") == "pending")
  {
    Datetime now;
    Datetime wait (get_date ("wait"));
    if (wait > now)
      return true;
  }

  return false;
}

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
      // Not using Pig to parse here (which would be idiomatic), because we
      // don't need to differentiate betwen utf-8 and normal characters.
      // Pig's scanning the string can be expensive.
      auto ending_bracket = input.find_last_of (']');
      if (ending_bracket != std::string::npos)
      {
        std::string line = input.substr(1, ending_bracket);

        if (line.length () == 0)
          throw std::string ("Empty record in input.");

        Pig attLine (line);
        std::string name;
        std::string value;
        while (!attLine.eos ())
        {
          if (attLine.getUntilAscii (':', name) &&
              attLine.skip (':')                &&
              attLine.getQuoted ('"', value))
          {
#ifdef PRODUCT_TASKWARRIOR
            legacyAttributeMap (name);
#endif

            if (isAnnotationAttr (name))
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

  // for compatibility, include all tags in `tags` as `tag_..` attributes
  if (data.find ("tags") != data.end ()) {
    for (auto& tag : split(data["tags"], ',')) {
      data[tag2Attr(tag)] = "x";
    }
  }
  // ..and similarly, update `tags` to match the `tag_..` attributes
  fixTagsAttribute();

  // same for `depends` / `dep_..`
  if (data.find ("depends") != data.end ()) {
    for (auto& dep : split(data["depends"], ',')) {
      data[dep2Attr(dep)] = "x";
    }
  }
  fixDependsAttribute();

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
    if (type != "")
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
        set (i.first, text == "" ? "" : d.toEpochString ());
      }

      // Tags are an array of JSON strings.
      else if (i.first == "tags" && i.second->type() == json::j_array)
      {
        auto tags = (json::array*)i.second;
        for (auto& t : tags->_data)
        {
          auto tag = (json::string*)t;
          addTag (tag->_data);
        }
      }

      // Dependencies can be exported as an array of strings.
      // 2016-02-21: This will be the only option in future releases.
      //             See other 2016-02-21 comments for details.
      else if (i.first == "depends" && i.second->type() == json::j_array)
      {
        auto deps = (json::array*)i.second;
        for (auto& t : deps->_data)
        {
          auto dep = (json::string*)t;
          addDependency (dep->_data);
        }
      }

      // Dependencies can be exported as a single comma-separated string.
      // 2016-02-21: Deprecated - see other 2016-02-21 comments for details.
      else if (i.first == "depends" && i.second->type() == json::j_string)
      {
        auto deps = (json::string*)i.second;

        // Fix for issue#2689: taskserver sometimes encodes the depends
        // property as a string of the format `[\"uuid\",\"uuid\"]`
        // The string includes the backslash-escaped `"` characters, making
        // it invalid JSON.  Since we know the characters we're looking for,
        // we'll just filter out everything else.
        std::string deps_str = deps->_data;
        if (deps_str.front () == '[' && deps_str.back () == ']') {
          std::string filtered;
		  for (auto &c: deps_str) {
			if ((c >= '0' && c <= '9') ||
				(c >= 'a' && c <= 'f') ||
				c == ',' || c == '-') {
			  filtered.push_back(c);
			}
		  }
		  deps_str = filtered;
        }
        auto uuids = split (deps_str, ',');

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

        // Fail if 'annotations' is not an array
        if (i.second->type() != json::j_array) {
            throw format ("Annotations is malformed: {1}", i.second->dump ());
        }

        auto atts = (json::array*)i.second;
        for (auto& annotations : atts->_data)
        {
          auto annotation = (json::object*)annotations;

          // Extract description. Fail if not present.
          auto what = (json::string*)annotation->_data["description"];
          if (! what) {
            annotation->_data.erase ("description");  // Erase NULL description inserted by failed lookup above
            throw format ("Annotation is missing a description: {1}", annotation->dump ());
          }

          // Extract 64-bit annotation entry value
          // Time travelers from 2038, we have your back.
          long long ann_timestamp;

          // Extract entry. Use current time if not present.
          auto when = (json::string*)annotation->_data["entry"];
          if (when)
            ann_timestamp = (long long) (Datetime (when->_data).toEpoch ());
          else {
            annotation->_data.erase ("entry");  // Erase NULL entry inserted by failed lookup above
            ann_timestamp = (long long) (Datetime ().toEpoch ());
          }

          std::stringstream name;
          name << "annotation_" << ann_timestamp;

          // Increment the entry timestamp in case of a conflict. Same
          // behaviour as CmdAnnotate.
          while (annos.find(name.str ()) != annos.end ())
          {
              name.str ("");  // Clear
              ann_timestamp++;
              name << "annotation_" << ann_timestamp;
          }

          annos.emplace (name.str (), json::decode (what->_data));
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
    if (type == "")
      type = "string";

    // If there is a value.
    if (it.second != "")
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

    // Tags and dependencies are handled below
    if (i.first == "tags" || isTagAttr (i.first))
      continue;
    if (i.first == "depends" || isDepAttr (i.first))
      continue;

    // If value is an empty string, do not ever output it
    if (i.second == "")
        continue;

    if (attributes_written)
      out << ',';

    std::string type = Task::attributes[i.first];
    if (type == "")
      type = "string";

    // Date fields are written as ISO 8601.
    if (type == "date")
    {
      Datetime d (i.second);
      out << '"'
          << (i.first == "modification" ? "modified" : i.first)
          << "\":\""
          // Date was deleted, do not export parsed empty string
          << (i.second == "" ? "" : d.toISO ())
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
        out << R"({"entry":")"
            << d.toISO ()
            << R"(","description":")"
            << json::encode (i.second)
            << "\"}";

        ++annotations_written;
      }
    }

    out << ']';
  }

  auto tags = getTags();
  if (tags.size() > 0)
  {
    out << ','
        << "\"tags\":[";

    int count = 0;
    for (const auto& tag : tags)
    {
      if (count++)
        out << ',';

      out << '"' << tag << '"';
    }

    out << ']';
    ++attributes_written;
  }

  auto depends = getDependencyUUIDs ();
  if (depends.size() > 0)
  {
    out << ','
        << "\"depends\":[";

    int count = 0;
    for (const auto& dep : depends)
    {
      if (count++)
        out << ',';

      out << '"' << dep << '"';
    }

    out << ']';
    ++attributes_written;
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
  if (uuid == "")
    throw format ("Could not create a dependency on task {1} - not found.", depid);

  // the addDependency(&std::string) overload will check this, too, but here we
  // can give an more natural error message containing the id the user
  // provided.
  if (hasDependency (uuid))
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

  if (hasDependency (uuid))
  {
#ifdef PRODUCT_TASKWARRIOR
    Context::getContext ().footnote (format ("Task {1} already depends on task {2}.", get ("uuid"), uuid));
#endif
    return;
  }

  // Store the dependency.
  set (dep2Attr (uuid), "x");

  // Prevent circular dependencies.
#ifdef PRODUCT_TASKWARRIOR
  if (dependencyIsCircular (*this))
    throw std::string ("Circular dependency detected and disallowed.");
#endif

  recalc_urgency = true;
  fixDependsAttribute();
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
void Task::removeDependency (int id)
{
  std::string uuid = Context::getContext ().tdb2.pending.uuid (id);

  // The removeDependency(std::string&) method will check this too, but here we
  // can give a more natural error message containing the id provided by the user
  if (uuid == "" || !has (dep2Attr (uuid)))
    throw format ("Could not delete a dependency on task {1} - not found.", id);
  removeDependency (uuid);
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeDependency (const std::string& uuid)
{
  auto depattr = dep2Attr (uuid);
  if (has (depattr))
    remove (depattr);
  else
    throw format ("Could not delete a dependency on task {1} - not found.", uuid);

  recalc_urgency = true;
  fixDependsAttribute();
}

////////////////////////////////////////////////////////////////////////////////
bool Task::hasDependency (const std::string& uuid) const
{
  auto depattr = dep2Attr (uuid);
  return has (depattr);
}

////////////////////////////////////////////////////////////////////////////////
std::vector <int> Task::getDependencyIDs () const
{
  std::vector <int> ids;
  for (auto& attr : all ()) {
    if (!isDepAttr (attr))
      continue;
    auto dep = attr2Dep (attr);
    ids.push_back (Context::getContext ().tdb2.pending.id (dep));
  }

  return ids;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Task::getDependencyUUIDs () const
{
  std::vector <std::string> uuids;
  for (auto& attr : all ()) {
    if (!isDepAttr (attr))
      continue;
    auto dep = attr2Dep (attr);
    uuids.push_back (dep);
  }

  return uuids;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <Task> Task::getDependencyTasks () const
{
  auto uuids = getDependencyUUIDs ();

  // NOTE: this may seem inefficient, but note that `TDB2::get` performs a
  // linear search on each invocation, so scanning *once* is quite a bit more
  // efficient.
  std::vector <Task> blocking;
  if (uuids.size() > 0)
    for (auto& it : Context::getContext ().tdb2.pending.get_tasks ())
      if (it.getStatus () != Task::completed &&
          it.getStatus () != Task::deleted   &&
          std::find (uuids.begin (), uuids.end (), it.get ("uuid")) != uuids.end ())
        blocking.push_back (it);

  return blocking;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <Task> Task::getBlockedTasks () const
{
  auto uuid = get ("uuid");

  std::vector <Task> blocked;
  for (auto& it : Context::getContext ().tdb2.pending.get_tasks ())
    if (it.getStatus () != Task::completed &&
        it.getStatus () != Task::deleted   &&
        it.hasDependency (uuid))
      blocked.push_back (it);

  return blocked;
}
#endif

////////////////////////////////////////////////////////////////////////////////
int Task::getTagCount () const
{
  auto count = 0;
  for (auto& attr : data) {
    if (isTagAttr (attr.first)) {
      count++;
    }
  }
  return count;
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
    // NOTE: This list should be kept synchronized with:
    // * the list in CmdTags.cpp for the _tags command.
    // * the list in CmdInfo.cpp for the info command.
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
    if (tag == "TAGGED")    return getTagCount() > 0;
    if (tag == "PARENT")    return has ("mask") || has ("last");       // 2017-01-07: Deprecated in 2.6.0
    if (tag == "TEMPLATE")  return has ("last") || has ("mask");
    if (tag == "WAITING")   return is_waiting ();
    if (tag == "PENDING")   return getStatus () == Task::pending;
    if (tag == "COMPLETED") return getStatus () == Task::completed;
    if (tag == "DELETED")   return getStatus () == Task::deleted;
#ifdef PRODUCT_TASKWARRIOR
    if (tag == "UDA")       return is_udaPresent ();
    if (tag == "ORPHAN")    return is_orphanPresent ();
    if (tag == "LATEST")    return id == Context::getContext ().tdb2.latest_id ();
#endif
    if (tag == "PROJECT")   return has ("project");
    if (tag == "PRIORITY")  return has ("priority");
  }

  // Concrete tags.
  if (has (tag2Attr (tag)))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Task::addTag (const std::string& tag)
{
  auto attr = tag2Attr (tag);
  if (!has (attr)) {
    set (attr, "x");
    recalc_urgency = true;
    fixTagsAttribute();
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::setTags (const std::vector <std::string>& tags)
{
  auto existing = getTags();

  // edit in-place, determining which should be
  // added and which should be removed
  std::vector <std::string> toAdd;
  std::vector <std::string> toRemove;

  for (auto& tag : tags) {
    if (std::find (existing.begin (), existing.end (), tag) == existing.end ())
      toAdd.push_back(tag);
  }

  for (auto& tag : getTags ()) {
    if (std::find (tags.begin (), tags.end (), tag) == tags.end ()) {
      toRemove.push_back (tag);
    }
  }

  for (auto& tag : toRemove) {
    removeTag (tag);
  }
  for (auto& tag : toAdd) {
    addTag (tag);
  }

  // (note: addTag / removeTag took care of recalculating urgency)
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Task::getTags () const
{
  std::vector <std::string> tags;

  for (auto& attr : data) {
    if (!isTagAttr (attr.first)) {
      continue;
    }
    auto tag = attr2Tag (attr.first);
    tags.push_back (tag);
  }

  return tags;
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeTag (const std::string& tag)
{
  auto attr = tag2Attr (tag);
  if (has (attr)) {
    data.erase (attr);
    recalc_urgency = true;
    fixTagsAttribute();
  }
}

////////////////////////////////////////////////////////////////////////////////
void Task::fixTagsAttribute ()
{
  // Fix up the old `tags` attribute to match the `tags_..` attributes (or
  // remove it if there are no tags)
  auto tags = getTags ();
  if (tags.size () > 0) {
    set ("tags", join (",", tags));
  } else {
    remove ("tags");
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Task::isTagAttr(const std::string& attr)
{
  return attr.compare(0, 5, "tags_") == 0;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Task::tag2Attr (const std::string& tag) const
{
  std::stringstream tag_attr;
  tag_attr << "tags_" << tag;
  return tag_attr.str();
}

////////////////////////////////////////////////////////////////////////////////
const std::string Task::attr2Tag (const std::string& attr) const
{
  assert (isTagAttr (attr));
  return attr.substr(5);
}

////////////////////////////////////////////////////////////////////////////////
void Task::fixDependsAttribute ()
{
  // Fix up the old `depends` attribute to match the `dep_..` attributes (or
  // remove it if there are no deps)
  auto deps = getDependencyUUIDs ();
  if (deps.size () > 0) {
    set ("depends", join (",", deps));
  } else {
    remove ("depends");
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Task::isDepAttr(const std::string& attr)
{
  return attr.compare(0, 4, "dep_") == 0;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Task::dep2Attr (const std::string& tag) const
{
  std::stringstream tag_attr;
  tag_attr << "dep_" << tag;
  return tag_attr.str();
}

////////////////////////////////////////////////////////////////////////////////
const std::string Task::attr2Dep (const std::string& attr) const
{
  assert (isDepAttr (attr));
  return attr.substr(4);
}

////////////////////////////////////////////////////////////////////////////////
bool Task::isAnnotationAttr(const std::string& attr)
{
  return attr.compare(0, 11, "annotation_") == 0;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
// A UDA Orphan is an attribute that is not represented in context.columns.
std::vector <std::string> Task::getUDAOrphans () const
{
  std::vector <std::string> orphans;
  for (auto& it : data)
    if (Context::getContext ().columns.find (it.first) == Context::getContext ().columns.end ())
      if (not (isAnnotationAttr (it.first) || isTagAttr (it.first) || isDepAttr (it.first)))
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
  if (get ("status") != "")
    status = getStatus ();

  // 1) Provide missing attributes where possible
  // Provide a UUID if necessary. Validate if present.
  std::string uid = get ("uuid");
  if (has ("uuid") && uid != "")
  {
    Lexer lex (uid);
    std::string token;
    Lexer::Type type;
    if (! lex.isUUID (token, type, true))
      throw format ("Not a valid UUID '{1}'.", uid);
  }
  else
    set ("uuid", uuid ());

  // TODO Obsolete remove for 3.0.0
  // Recurring tasks get a special status.
  if (status == Task::pending                     &&
      has ("due")                                 &&
      has ("recur")                               &&
      (! has ("parent") || get ("parent") == "")  &&
      (! has ("template") || get ("template") == ""))
  {
    status = Task::recurring;
  }
/*
  // TODO Add for 3.0.0
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
           get ("wait") != "")
    status = Task::waiting;

  // By default, tasks are pending.
  else if (! has ("status") || get ("status") == "")
    status = Task::pending;

  // Default to 'periodic' type recurrence.
  if (status == Task::recurring &&
      (! has ("rtype") || get ("rtype") == ""))
  {
    set ("rtype", "periodic");
  }

  // Store the derived status.
  setStatus (status);

#ifdef PRODUCT_TASKWARRIOR
  // Provide an entry date unless user already specified one.
  if (! has ("entry") || get ("entry") == "")
    setAsNow ("entry");

  // Completed tasks need an end date, so inherit the entry date.
  if ((status == Task::completed || status == Task::deleted) &&
      (! has ("end") || get ("end") == ""))
    setAsNow ("end");

  // Pending tasks cannot have an end date, remove if present
  if ((status == Task::pending) && (get ("end") != ""))
    remove ("end");

  // Provide an entry date unless user already specified one.
  if (! has ("modified") || get ("modified") == "")
    setAsNow ("modified");

  if (applyDefault && (! has ("parent") || get ("parent") == ""))
  {
    // Override with default.project, if not specified.
    if (Task::defaultProject != "" &&
        ! has ("project"))
    {
      if (Context::getContext ().columns["project"]->validate (Task::defaultProject))
        set ("project", Task::defaultProject);
    }

    // Override with default.due, if not specified.
    if (Task::defaultDue != "" &&
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
    if (Task::defaultScheduled != "" &&
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

    if (udas.size ())
    {
      // For each of those, setup the default value on the task now,
      // of course only if we don't have one on the command line already
      for (auto& uda : udas)
      {
        std::string defVal= Context::getContext ().config.get ("uda." + uda + ".default");

        // If the default is empty, or we already have a value, skip it
        if (defVal != "" && get (uda) == "")
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
  else if (get ("description") == "")
    throw std::string ("Cannot add a task that is blank.");

  // Cannot have a recur frequency with no due date - when would it recur?
  if (has ("recur") && (! has ("due") || get ("due") == ""))
    throw std::string ("A recurring task must also have a 'due' date.");

  // Recur durations must be valid.
  if (has ("recur"))
  {
    std::string value = get ("recur");
    if (value != "")
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
const std::string Task::encode (const std::string& value) const
{
  auto modified = str_replace (value,    "[", "&open;");
  return          str_replace (modified, "]", "&close;");
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   [  <- &open;
//   ]  <- &close;
const std::string Task::decode (const std::string& value) const
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

  const std::string taskProjectName = get("project");
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

          if (taskProjectName == project ||
              taskProjectName.find(project + '.') == 0)
          {
            value += var.second;
          }
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
  // Calling getBlockedTasks is rather expensive.
  // It is called recursively for each dependency in the chain here.
  for (auto& task : getBlockedTasks ())
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
  if (is_waiting ())
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

  // while reading the parse tree, consider DOM references in the context of
  // this task
  auto currentTask = Context::getContext ().withCurrentTask(this);

  // Need this for later comparison.
  auto originalStatus = getStatus ();

  std::string text = "";
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
        if (value == ""     ||
            value == "''"   ||
            value == "\"\"")
        {
          // Special case: Handle bulk removal of 'tags' and 'depends" virtual
          // attributes
          if (name == "depends")
          {
            for (auto dep: getDependencyUUIDs ())
              removeDependency(dep);
          }
          else if (name == "tags")
          {
            for (auto tag: getTags ())
              removeTag(tag);
          }

          // ::composeF4 will skip if the value is blank, but the presence of
          // the attribute will prevent ::validate from applying defaults.
          if ((has (name) && get (name) != "") ||
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
        if (text != "")
          text += ' ';
        text += a.attribute ("raw");
      }
    }
  }

  // Task::modType determines what happens to the WORD arguments, if there are
  //  any.
  if (text != "")
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
    Context::getContext ().footnote (format ("Note: Modified task {1} is {2}. You may wish to make this task pending with: task {3} modify status:pending", uuid, get ("status"), uuid));
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Compare this task to another and summarize the differences for display, in
// the future tense ("Foo will be set to ..").
std::string Task::diff (const Task& after) const
{
  // Attributes are all there is, so figure the different attribute names
  // between this (before) and after.
  std::vector <std::string> beforeAtts;
  for (auto& att : data)
    beforeAtts.push_back (att.first);

  std::vector <std::string> afterAtts;
  for (auto& att : after.data)
    afterAtts.push_back (att.first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  // Now start generating a description of the differences.
  std::stringstream out;
  for (auto& name : beforeOnly)
  {
    if (isAnnotationAttr (name))
    {
      out << "  - "
          << format ("Annotation {1} will be removed.", name)
          << "\n";
    }
    else if (isTagAttr (name))
    {
      out << "  - "
          << format ("Tag {1} will be removed.", attr2Tag (name))
          << "\n";
    }
    else if (isDepAttr (name))
    {
      out << "  - "
          << format ("Depenency on {1} will be removed.", attr2Dep (name))
          << "\n";
    }
    else if (name == "depends" || name == "tags")
    {
      // do nothing for legacy attributes
    }
    else
    {
      out << "  - "
          << format ("{1} will be deleted.", Lexer::ucFirst (name))
          << "\n";
    }
  }

  for (auto& name : afterOnly)
  {
    if (isAnnotationAttr (name))
    {
      out << format ("Annotation of {1} will be added.\n", after.get (name));
    }
    else if (isTagAttr (name))
    {
      out << format ("Tag {1} will be added.\n", attr2Tag (name));
    }
    else if (isDepAttr (name))
    {
      out << format ("Dependency on {1} will be added.\n", attr2Dep (name));
    }
    else if (name == "depends" || name == "tags")
    {
      // do nothing for legacy attributes
    }
    else
      out << "  - "
          << format ("{1} will be set to '{2}'.",
                     Lexer::ucFirst (name),
                     renderAttribute (name, after.get (name)))
          << "\n";
  }

  for (auto& name : beforeAtts)
  {
    // Ignore UUID differences, and find values that changed, but are not also
    // in the beforeOnly and afterOnly lists, which have been handled above..
    if (name              != "uuid" &&
        get (name)        != after.get (name) &&
        std::find (beforeOnly.begin (), beforeOnly.end (), name) == beforeOnly.end () &&
        std::find (afterOnly.begin (),  afterOnly.end (),  name) == afterOnly.end ())
    {
      if (name == "depends" || name == "tags")
      {
        // do nothing for legacy attributes
      }
      else if (isTagAttr (name) || isDepAttr (name))
      {
        // ignore new attributes
      }
      else if (isAnnotationAttr (name))
      {
        out << format ("Annotation will be changed to {1}.\n", after.get (name));
      }
      else
        out << "  - "
            << format ("{1} will be changed from '{2}' to '{3}'.",
                       Lexer::ucFirst (name),
                       renderAttribute (name, get (name)),
                       renderAttribute (name, after.get (name)))
            << "\n";
    }
  }

  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << "  - No changes will be made.\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Similar to diff, but formatted for inclusion in the output of the info command
std::string Task::diffForInfo (
  const Task& after,
  const std::string& dateformat,
  long& last_timestamp,
  const long current_timestamp) const
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  for (auto& att : data)
    beforeAtts.push_back (att.first);

  std::vector <std::string> afterAtts;
  for (auto& att : after.data)
    afterAtts.push_back (att.first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  // Now start generating a description of the differences.
  std::stringstream out;
  for (auto& name : beforeOnly)
  {
    if (isAnnotationAttr (name))
    {
      out << format ("Annotation '{1}' deleted.\n", get (name));
    }
    else if (isTagAttr (name))
    {
      out << format ("Tag '{1}' deleted.\n", attr2Tag(name));
    }
    else if (isDepAttr (name))
    {
      out << format ("Dependency on '{1}' deleted.\n", attr2Dep(name));
    }
    else if (name == "depends" || name == "tags")
    {
      // do nothing for legacy attributes
    }
    else if (name == "start")
    {
      Datetime started (get ("start"));
      Datetime stopped;

      if (after.has ("end"))
        // Task was marked as finished, use end time
        stopped = Datetime (after.get ("end"));
      else
        // Start attribute was removed, use modification time
        stopped = Datetime (current_timestamp);

      out << format ("{1} deleted (duration: {2}).",
                     Lexer::ucFirst (name),
                     Duration (stopped - started).format ())
          << "\n";
    }
    else
    {
      out << format ("{1} deleted.\n", Lexer::ucFirst (name));
    }
  }

  for (auto& name : afterOnly)
  {
    if (isAnnotationAttr (name))
    {
      out << format ("Annotation of '{1}' added.\n", after.get (name));
    }
    else if (isTagAttr (name))
    {
      out << format ("Tag '{1}' added.\n", attr2Tag (name));
    }
    else if (isDepAttr (name))
    {
      out << format ("Dependency on '{1}' added.\n", attr2Dep (name));
    }
    else if (name == "depends" || name == "tags")
    {
      // do nothing for legacy attributes
    }
    else
    {
      if (name == "start")
          last_timestamp = current_timestamp;

      out << format ("{1} set to '{2}'.",
                     Lexer::ucFirst (name),
                     renderAttribute (name, after.get (name), dateformat))
          << "\n";
    }
  }

  for (auto& name : beforeAtts)
    if (name              != "uuid" &&
        name              != "modified" &&
        get (name)        != after.get (name) &&
        get (name)        != "" &&
        after.get (name)  != "")
    {
      if (name == "depends" || name == "tags")
      {
        // do nothing for legacy attributes
      }
      else if (isTagAttr (name) || isDepAttr (name))
      {
        // ignore new attributes
      }
      else if (isAnnotationAttr (name))
      {
        out << format ("Annotation changed to '{1}'.\n", after.get (name));
      }
      else
        out << format ("{1} changed from '{2}' to '{3}'.",
                       Lexer::ucFirst (name),
                       renderAttribute (name, get (name), dateformat),
                       renderAttribute (name, after.get (name), dateformat))
            << "\n";
    }

  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << "No changes made.\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Similar to diff, but formatted as a side-by-side table for an Undo preview
Table Task::diffForUndoSide (
  const Task& after) const
{
  // Set the colors.
  Color color_red   (Context::getContext ().color () ? Context::getContext ().config.get ("color.undo.before") : "");
  Color color_green (Context::getContext ().color () ? Context::getContext ().config.get ("color.undo.after") : "");

  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  Table view;
  view.width (Context::getContext ().getWidth ());
  view.intraPadding (2);
  view.add ("");
  view.add ("Prior Values");
  view.add ("Current Values");
  setHeaderUnderline (view);

  if (!is_empty ())
  {
    const Task &before = *this;

    std::vector <std::string> beforeAtts;
    for (auto& att : before.data)
      beforeAtts.push_back (att.first);

    std::vector <std::string> afterAtts;
    for (auto& att : after.data)
      afterAtts.push_back (att.first);

    std::vector <std::string> beforeOnly;
    std::vector <std::string> afterOnly;
    listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

    int row;
    for (auto& name : beforeOnly)
    {
      row = view.addRow ();
      view.set (row, 0, name);
      view.set (row, 1, renderAttribute (name, before.get (name)), color_red);
    }

    for (auto& att : before.data)
    {
      std::string priorValue   = before.get (att.first);
      std::string currentValue = after.get  (att.first);

      if (currentValue != "")
      {
        row = view.addRow ();
        view.set (row, 0, att.first);
        view.set (row, 1, renderAttribute (att.first, priorValue),
                  (priorValue != currentValue ? color_red : Color ()));
        view.set (row, 2, renderAttribute (att.first, currentValue),
                  (priorValue != currentValue ? color_green : Color ()));
      }
    }

    for (auto& name : afterOnly)
    {
      row = view.addRow ();
      view.set (row, 0, name);
      view.set (row, 2, renderAttribute (name, after.get (name)), color_green);
    }
  }
  else
  {
    int row;
    for (auto& att : after.data)
    {
      row = view.addRow ();
      view.set (row, 0, att.first);
      view.set (row, 2, renderAttribute (att.first, after.get (att.first)), color_green);
    }
  }

  return view;
}

////////////////////////////////////////////////////////////////////////////////
// Similar to diff, but formatted as a diff for an Undo preview
Table Task::diffForUndoPatch (
  const Task& after,
  const Datetime& lastChange) const
{
  // This style looks like this:
  //  --- before    2009-07-04 00:00:25.000000000 +0200
  //  +++ after    2009-07-04 00:00:45.000000000 +0200
  //
  // - name: old           // att deleted
  // + name:
  //
  // - name: old           // att changed
  // + name: new
  //
  // - name:
  // + name: new           // att added
  //

  // Set the colors.
  Color color_red   (Context::getContext ().color () ? Context::getContext ().config.get ("color.undo.before") : "");
  Color color_green (Context::getContext ().color () ? Context::getContext ().config.get ("color.undo.after") : "");

  const Task &before = *this;

  // Generate table header.
  Table view;
  view.width (Context::getContext ().getWidth ());
  view.intraPadding (2);
  view.add ("");
  view.add ("");

  int row = view.addRow ();
  view.set (row, 0, "--- previous state", color_red);
  view.set (row, 1, "Undo will restore this state", color_red);

  row = view.addRow ();
  view.set (row, 0, "+++ current state ", color_green);
  view.set (row, 1, format ("Change made {1}",
                            lastChange.toString (Context::getContext ().config.get ("dateformat"))),
                    color_green);

  view.addRow ();

  // Add rows to table showing diffs.
  std::vector <std::string> all = Context::getContext ().getColumns ();

  // Now factor in the annotation attributes.
  for (auto& it : before.data)
    if (it.first.substr (0, 11) == "annotation_")
      all.push_back (it.first);

  for (auto& it : after.data)
    if (it.first.substr (0, 11) == "annotation_")
      all.push_back (it.first);

  // Now render all the attributes.
  std::sort (all.begin (), all.end ());

  std::string before_att;
  std::string after_att;
  std::string last_att;
  for (auto& a : all)
  {
    if (a != last_att)  // Skip duplicates.
    {
      last_att = a;

      before_att = before.get (a);
      after_att  = after.get (a);

      // Don't report different uuid.
      // Show nothing if values are the unchanged.
      if (a == "uuid" ||
          before_att == after_att)
      {
        // Show nothing - no point displaying that which did not change.

        // row = view.addRow ();
        // view.set (row, 0, *a + ":");
        // view.set (row, 1, before_att);
      }

      // Attribute deleted.
      else if (before_att != "" && after_att == "")
      {
        row = view.addRow ();
        view.set (row, 0, '-' + a + ':', color_red);
        view.set (row, 1, before_att, color_red);

        row = view.addRow ();
        view.set (row, 0, '+' + a + ':', color_green);
      }

      // Attribute added.
      else if (before_att == "" && after_att != "")
      {
        row = view.addRow ();
        view.set (row, 0, '-' + a + ':', color_red);

        row = view.addRow ();
        view.set (row, 0, '+' + a + ':', color_green);
        view.set (row, 1, after_att, color_green);
      }

      // Attribute changed.
      else
      {
        row = view.addRow ();
        view.set (row, 0, '-' + a + ':', color_red);
        view.set (row, 1, before_att, color_red);

        row = view.addRow ();
        view.set (row, 0, '+' + a + ':', color_green);
        view.set (row, 1, after_att, color_green);
      }
    }
  }

  return view;
}

////////////////////////////////////////////////////////////////////////////////
