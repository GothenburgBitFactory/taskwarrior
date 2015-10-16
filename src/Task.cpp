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
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <string>
#ifdef PRODUCT_TASKWARRIOR
#include <math.h>
#include <ctype.h>
#include <cfloat>
#endif
#include <algorithm>
#ifdef PRODUCT_TASKWARRIOR
#include <Context.h>
#include <Lexer.h>
#include <Nibbler.h>
#endif
#include <ISO8601.h>
#include <Task.h>
#ifdef PRODUCT_TASKWARRIOR
#include <RX.h>
#endif
#include <text.h>
#include <util.h>

#include <i18n.h>
#ifdef PRODUCT_TASKWARRIOR
#include <main.h>

#include <Eval.h>
#include <Variant.h>
#include <Filter.h>
#include <Dates.h>

#define APPROACHING_INFINITY 1000   // Close enough.  This isn't rocket surgery.

extern Context context;
extern Task& contextTask;

static const float epsilon = 0.000001;
#endif

std::string Task::defaultProject  = "";
std::string Task::defaultDue      = "";
bool Task::searchCaseSensitive    = true;
bool Task::regex                  = false;
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
Task::Task ()
: id (0)
, urgency_value (0.0)
, recalc_urgency (true)
, is_blocked (false)
, is_blocking (false)
, annotation_count (0)
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
    id               = other.id;
    urgency_value    = other.urgency_value;
    recalc_urgency   = other.recalc_urgency;
    is_blocked       = other.is_blocked;
    is_blocking      = other.is_blocking;
    annotation_count = other.annotation_count;
  }

  return *this;
}

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
  if (size () != other.size ())
    return false;

  for (auto& i : *this)
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

  throw format (STRING_ERROR_BAD_STATUS, input);
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
void Task::setAsNow (const std::string& att)
{
  char now[16];
  sprintf (now, "%u", (unsigned int) time (NULL));
  set (att, now);

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::has (const std::string& name) const
{
  auto i = this->find (name);
  if (i != this->end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Task::all ()
{
  std::vector <std::string> all;
  for (auto i = this->begin (); i != this->end (); ++i)
    all.push_back (i->first);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Task::get (const std::string& name) const
{
  auto i = this->find (name);
  if (i != this->end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
const std::string& Task::get_ref (const std::string& name) const
{
  auto i = this->find (name);
  if (i != this->end ())
    return i->second;

  return dummy;
}

////////////////////////////////////////////////////////////////////////////////
int Task::get_int (const std::string& name) const
{
  auto i = this->find (name);
  if (i != this->end ())
    return strtol (i->second.c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
unsigned long Task::get_ulong (const std::string& name) const
{
  auto i = this->find (name);
  if (i != this->end ())
    return strtoul (i->second.c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
float Task::get_float (const std::string& name) const
{
  auto i = this->find (name);
  if (i != this->end ())
    return strtof (i->second.c_str (), NULL);

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
time_t Task::get_date (const std::string& name) const
{
  auto i = this->find (name);
  if (i != this->end ())
    return (time_t) strtoul (i->second.c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Task::set (const std::string& name, const std::string& value)
{
  (*this)[name] = json::decode (value);

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
  auto it = this->find (name);
  if (it != this->end ())
  {
    this->erase (it);
    recalc_urgency = true;
  }
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
// Determines status of a date attribute.
Task::dateState Task::getDateState (const std::string& name) const
{
  std::string value = get (name);
  if (value.length ())
  {
    ISO8601d reference (value);
    ISO8601d now;
    ISO8601d today ("today");

    if (reference < today)
      return dateBeforeToday;

    if (reference.sameDay (now))
    {
      if (reference < now)
        return dateEarlierToday;
      else
        return dateLaterToday;
    }

    int imminentperiod = context.config.getInteger ("due");
    if (imminentperiod == 0)
      return dateAfterToday;

    ISO8601d imminentDay = today + imminentperiod * 86400;
    if (reference < imminentDay)
      return dateAfterToday;
  }

  return dateNotDue;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
// Ready means pending, not blocked and either not scheduled or scheduled before
// now.
bool Task::is_ready () const
{
  return getStatus () == Task::pending &&
         ! is_blocked                  &&
         (! has ("scheduled")          ||
          ISO8601d ("now").operator> (get_date ("scheduled")));
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
      if (ISO8601d ("yesterday").sameDay (get_date ("due")))
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
      if (ISO8601d ("tomorrow").sameDay (get_date ("due")))
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
      ISO8601d due (get_date ("due"));
      if (due >= ISO8601d ("socw") &&
          due <= ISO8601d ("eocw"))
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
      ISO8601d due (get_date ("due"));
      if (due >= ISO8601d ("socm") &&
          due <= ISO8601d ("eocm"))
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
      ISO8601d now;
      ISO8601d due (get_date ("due"));
      if (now.year () == due.year ())
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_udaPresent () const
{
  for (auto& col : context.columns)
    if (col.first.substr (0, 11) != "annotation_")
      if (col.second->is_uda () &&
          has (col.first))
        return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Task::is_orphanPresent () const
{
  for (auto& att : *this)
    if (att.first.substr (0, 11) != "annotation_")
      if (context.columns.find (att.first) == context.columns.end ())
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
  // TODO Is this simply a 'chomp'?
  std::string copy;
  if (input[input.length () - 1] == '\n')
    copy = input.substr (0, input.length () - 1);
  else
    copy = input;

  try
  {
    // File format version 4, from 2009-5-16 - now, v1.7.1+
    // This is the parse format tried first, because it is most used.
    clear ();

    if (copy[0] == '[')
    {
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
            legacyAttributeMap (name);

            if (name.substr (0, 11) == "annotation_")
              ++annotation_count;

            (*this)[name] = decode (json::decode (value));
          }

          nl.skip (' ');
        }

        std::string remainder;
        nl.getUntilEOS (remainder);
        if (remainder.length ())
          throw std::string (STRING_RECORD_JUNK_AT_EOL);
      }
    }
    else if (copy[0] == '{')
      parseJSON (copy);
    else
      throw std::string (STRING_RECORD_NOT_FF4);
  }

  catch (const std::string&)
  {
    parseLegacy (copy);
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
        ISO8601d d (unquoteText (i.second->dump ()));
        set ("modified", d.toEpochString ());
      }

      // Dates are converted from ISO to epoch.
      else if (type == "date")
      {
        std::string text = unquoteText (i.second->dump ());
        ISO8601d d (text);
        set (i.first, text == "" ? "" : d.toEpochString ());
      }

      // Tags are an array of JSON strings.
      else if (i.first == "tags" && i.second->type() == json::j_array)
      {
        json::array* tags = (json::array*)i.second;
        for (auto& t : tags->_data)
        {
          json::string* tag = (json::string*)t;
          addTag (tag->_data);
        }
      }
      // This is a temporary measure to allow Mirakel sync, and will be removed
      // in a future release.
      else if (i.first == "tags" && i.second->type() == json::j_string)
      {
        json::string* tag = (json::string*)i.second;
        addTag (tag->_data);
      }

      // Dependencies can be exported as a single comma-separated string, or as
      // an array of strings.
      else if (i.first == "depends" && i.second->type() == json::j_array)
      {
        json::array* tags = (json::array*)i.second;
        for (auto& t : tags->_data)
        {
          json::string* tag = (json::string*)t;
          addDependency (tag->_data);
        }
      }

      // Strings are decoded.
      else if (type == "string")
        set (i.first, json::decode (unquoteText (i.second->dump ())));

      // Other types are simply added.
      else
        set (i.first, unquoteText (i.second->dump ()));
    }

    // UDA orphans and annotations do not have columns.
    else
    {
      // Annotations are an array of JSON objects with 'entry' and
      // 'description' values and must be converted.
      if (i.first == "annotations")
      {
        std::map <std::string, std::string> annos;

        json::array* atts = (json::array*)i.second;
        for (auto& annotations : atts->_data)
        {
          json::object* annotation = (json::object*)annotations;
          json::string* when = (json::string*)annotation->_data["entry"];
          json::string* what = (json::string*)annotation->_data["description"];

          if (! when)
            throw format (STRING_TASK_NO_ENTRY, root_obj->dump ());

          if (! what)
            throw format (STRING_TASK_NO_DESC, root_obj->dump ());

          std::string name = "annotation_" + ISO8601d (when->_data).toEpochString ();
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
        context.debug (message.str ());
#endif
        set (i.first, json::decode (unquoteText (i.second->dump ())));
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
  case 1: throw std::string (STRING_TASK_NO_FF1);

  // File format version 2, from 2008-1-1 - 2009-3-23, v0.9.3 - v1.5.0
  case 2: throw std::string (STRING_TASK_NO_FF2);

  // File format version 3, from 2009-3-23 - 2009-05-16, v1.6.0 - v1.7.1
  case 3: throw std::string (STRING_TASK_NO_FF3);

  // File format version 4, from 2009-05-16 - today, v1.7.1+
  case 4:
    break;

  default:
    std::stringstream message;
    message << "Invalid fileformat at line '"
            << line
            << "'";
    context.debug (message.str ());
    throw std::string (STRING_TASK_PARSE_UNREC_FF);
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
  for (auto it : *this)
  {
    if (it.second != "")
    {
      ff4 += (first ? "" : " ")
           + it.first
           + ":\""
           + encode (json::encode (it.second))
           + "\"";

      first = false;
    }
  }

  ff4 += "]";
  return ff4;
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::composeJSON (bool decorate /*= false*/) const
{
  std::stringstream out;
  out << "{";

  // ID inclusion is optional, but not a good idea, because it remains correct
  // only until the next gc.
  if (decorate)
    out << "\"id\":" << id << ",";

  // First the non-annotations.
  int attributes_written = 0;
  for (auto& i : *this)
  {
    // Annotations are not written out here.
    if (i.first.substr (0, 11) == "annotation_")
      continue;

    // If value is an empty string, do not ever output it
    if (i.second == "")
        continue;

    if (attributes_written)
      out << ",";

    std::string type = Task::attributes[i.first];
    if (type == "")
      type = "string";

    // Date fields are written as ISO 8601.
    if (type == "date")
    {
      ISO8601d d (i.second);
      out << "\""
          << (i.first == "modification" ? "modified" : i.first)
          << "\":\""
          // Date was deleted, do not export parsed empty string
          << (i.second == "" ? "" : d.toISO ())
          << "\"";

      ++attributes_written;
    }

/*
    else if (type == "duration")
    {
      // TODO Emit ISO8601d
    }
*/
    else if (type == "numeric")
    {
      out << "\""
          << i.first
          << "\":"
          << i.second;

      ++attributes_written;
    }

    // Tags are converted to an array.
    else if (i.first == "tags")
    {
      std::vector <std::string> tags;
      split (tags, i.second, ',');

      out << "\"tags\":[";

      for (auto i = tags.begin (); i != tags.end (); ++i)
      {
        if (i != tags.begin ())
          out << ",";

        out << "\"" << *i << "\"";
      }

      out << "]";
      ++attributes_written;
    }

    // Dependencies are an array by default.
    else if (i.first == "depends" &&
             context.config.getBoolean ("json.depends.array"))
    {
      std::vector <std::string> deps;
      split (deps, i.second, ',');

      out << "\"depends\":[";

      for (auto i = deps.begin (); i != deps.end (); ++i)
      {
        if (i != deps.begin ())
          out << ",";

        out << "\"" << *i << "\"";
      }

      out << "]";
      ++attributes_written;
    }

    // Everything else is a quoted value.
    else
    {
      out << "\""
          << i.first
          << "\":\""
          << json::encode (i.second)
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
    for (auto& i : *this)
    {
      if (i.first.substr (0, 11) == "annotation_")
      {
        if (annotations_written)
          out << ",";

        ISO8601d d (i.first.substr (11));
        out << "{\"entry\":\""
            << d.toISO ()
            << "\",\"description\":\""
            << json::encode (i.second)
            << "\"}";

        ++annotations_written;
      }
    }

    out << "]";
  }

#ifdef PRODUCT_TASKWARRIOR
  // Include urgency.
  if (decorate)
    out << ","
        << "\"urgency\":"
        << urgency_c ();
#endif

  out << "}";
  return out.str ();
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
  time_t now = time (NULL);
  std::string key;

  do
  {
    key = "annotation_" + format ((int) now);
    ++now;
  }
  while (has (key));

  (*this)[key] = json::decode (description);
  ++annotation_count;
  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeAnnotations ()
{
  // Erase old annotations.
  auto i = this->begin ();
  while (i != this->end ())
  {
    if (i->first.substr (0, 11) == "annotation_")
    {
      --annotation_count;
      this->erase (i++);
    }
    else
      i++;
  }

  recalc_urgency = true;
}

////////////////////////////////////////////////////////////////////////////////
void Task::getAnnotations (std::map <std::string, std::string>& annotations) const
{
  annotations.clear ();

  for (auto& ann : *this)
    if (ann.first.substr (0, 11) == "annotation_")
      annotations.insert (ann);
}

////////////////////////////////////////////////////////////////////////////////
void Task::setAnnotations (const std::map <std::string, std::string>& annotations)
{
  // Erase old annotations.
  removeAnnotations ();

  for (auto& anno : annotations)
    this->insert (anno);

  annotation_count = annotations.size ();
  recalc_urgency = true;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
void Task::addDependency (int id)
{
  // Check that id is resolvable.
  std::string uuid = context.tdb2.pending.uuid (id);
  if (uuid == "")
    throw format (STRING_TASK_DEPEND_MISS_CREA, id);

  std::string depends = get ("depends");
  if (depends.find (uuid) != std::string::npos)
  {
    context.footnote (format (STRING_TASK_DEPEND_DUP, this->id, id));
    return;
  }

  addDependency(uuid);
}

////////////////////////////////////////////////////////////////////////////////
void Task::addDependency (const std::string& uuid)
{
  if (uuid == get ("uuid"))
    throw std::string (STRING_TASK_DEPEND_ITSELF);

  // Store the dependency.
  std::string depends = get ("depends");
  if (depends != "")
  {
    // Check for extant dependency.
    if (depends.find (uuid) == std::string::npos)
      set ("depends", depends + "," + uuid);
    else
    {
      context.footnote (format (STRING_TASK_DEPEND_DUP, this->get ("uuid"), uuid));
      return;
    }
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

  auto i = std::find (deps.begin (), deps.end (), uuid);
  if (i != deps.end ())
  {
    deps.erase (i);
    std::string combined;
    join (combined, ",", deps);
    set ("depends", combined);
    recalc_urgency = true;
  }
  else
    throw format (STRING_TASK_DEPEND_MISS_DEL, uuid);
}

////////////////////////////////////////////////////////////////////////////////
void Task::removeDependency (int id)
{
  std::string depends = get ("depends");
  std::string uuid = context.tdb2.pending.uuid (id);
  if (uuid != "" && depends.find (uuid) != std::string::npos)
    removeDependency (uuid);
  else
    throw format (STRING_TASK_DEPEND_MISS_DEL, id);
}

////////////////////////////////////////////////////////////////////////////////
void Task::getDependencies (std::vector <int>& all) const
{
  std::vector <std::string> deps;
  split (deps, get ("depends"), ',');

  all.clear ();

  for (auto& dep : deps)
    all.push_back (context.tdb2.pending.id (dep));
}

////////////////////////////////////////////////////////////////////////////////
void Task::getDependencies (std::vector <std::string>& all) const
{
  all.clear ();
  split (all, get ("depends"), ',');
}
#endif

////////////////////////////////////////////////////////////////////////////////
int Task::getTagCount () const
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ',');

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
    if (tag == "DUETODAY")  return is_duetoday ();
    if (tag == "TODAY")     return is_duetoday ();
    if (tag == "YESTERDAY") return is_dueyesterday ();
    if (tag == "TOMORROW")  return is_duetomorrow ();
    if (tag == "OVERDUE")   return is_overdue ();
    if (tag == "WEEK")      return is_dueweek ();
    if (tag == "MONTH")     return is_duemonth ();
    if (tag == "YEAR")      return is_dueyear ();
#endif
    if (tag == "ACTIVE")    return has ("start");
    if (tag == "SCHEDULED") return has ("scheduled");
    if (tag == "CHILD")     return has ("parent");
    if (tag == "UNTIL")     return has ("until");
    if (tag == "ANNOTATED") return hasAnnotations ();
    if (tag == "TAGGED")    return has ("tags");
    if (tag == "PARENT")    return has ("mask");
    if (tag == "WAITING")   return get ("status") == "waiting";
    if (tag == "PENDING")   return get ("status") == "pending";
    if (tag == "COMPLETED") return get ("status") == "completed";
    if (tag == "DELETED")   return get ("status") == "deleted";
    if (tag == "UDA")       return is_udaPresent ();
    if (tag == "ORPHAN")    return is_orphanPresent ();
    if (tag == "LATEST")    return id == context.tdb2.latest_id ();
    if (tag == "PROJECT")   return has ("project");
    if (tag == "PRIORITY")  return has ("priority");
  }

  // Concrete tags.
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

  for (auto& tag : tags)
    addTag (tag);

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

  auto i = std::find (tags.begin (), tags.end (), tag);
  if (i != tags.end ())
  {
    tags.erase (i);
    std::string combined;
    join (combined, ",", tags);
    set ("tags", combined);
  }

  recalc_urgency = true;
}

#ifdef PRODUCT_TASKWARRIOR
////////////////////////////////////////////////////////////////////////////////
// A UDA Orphan is an attribute that is not represented in context.columns.
void Task::getUDAOrphans (std::vector <std::string>& names) const
{
  for (auto& it : *this)
    if (it.first.substr (0, 11) != "annotation_")
      if (context.columns.find (it.first) == context.columns.end ())
        names.push_back (it.first);
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
  std::map <std::string, std::string> annotations;
  getAnnotations (annotations);

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
        throw format (STRING_INFINITE_LOOP, APPROACHING_INFINITY);
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
      throw format (STRING_CMD_IMPORT_UUID_BAD, uid);
  }
  else
    set ("uuid", uuid ());

  // Recurring tasks get a special status.
  if (status == Task::pending &&
      has ("due")             &&
      has ("recur")           &&
      (! has ("parent") || get ("parent") == ""))
    status = Task::recurring;

  // Tasks with a wait: date get a special status.
  else if (status == Task::pending &&
           has ("wait")            &&
           get ("wait") != "")
    status = Task::waiting;

  // By default, tasks are pending.
  else if (! has ("status") || get ("status") == "")
    status = Task::pending;

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

  // Provide an entry date unless user already specified one.
  if (! has ("modified") || get ("modified") == "")
    setAsNow ("modified");

  if (applyDefault && (! has ("parent") || get ("parent") == ""))
  {
    // Override with default.project, if not specified.
    if (Task::defaultProject != "" &&
        ! has ("project"))
    {
      if (context.columns["project"]->validate (Task::defaultProject))
        set ("project", Task::defaultProject);
    }

    // Override with default.due, if not specified.
    if (Task::defaultDue != "" &&
        ! has ("due"))
    {
      if (context.columns["due"]->validate (Task::defaultDue))
      {
        ISO8601p dur (Task::defaultDue);
        if ((time_t) dur != 0)
          set ("due", (ISO8601d () + dur).toEpoch ());
        else
          set ("due", ISO8601d (Task::defaultDue).toEpoch ());
      }
    }

    // If a UDA has a default value in the configuration,
    // override with uda.(uda).default, if not specified.
    // Gather a list of all UDAs with a .default value
    std::vector <std::string> udas;
    for (auto& var : context.config)
    {
      if (var.first.substr (0, 4) == "uda." &&
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
        std::string defVal= context.config.get ("uda." + uda + ".default");

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
    throw std::string (STRING_TASK_VALID_DESC);
  else if (get ("description") == "")
    throw std::string (STRING_TASK_VALID_BLANK);

  // Cannot have a recur frequency with no due date - when would it recur?
  if (has ("recur") && (! has ("due") || get ("due") == ""))
    throw std::string (STRING_TASK_VALID_REC_DUE);

  // Recur durations must be valid.
  if (has ("recur"))
  {
    std::string value = get ("recur");
    if (value != "")
    {
      ISO8601p p;
      std::string::size_type i = 0;
      if (! p.parse (value, i))
        throw format (STRING_TASK_VALID_RECUR, value);
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
    ISO8601d date_left (get_date (left));
    ISO8601d date_right (get_date (right));

    // if date is zero, then it is being removed (e.g. "due: wait:1day")
    if (date_left > date_right && date_right.toEpoch () != 0)
      context.footnote (format (STRING_TASK_VALID_BEFORE, left, right));
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Encode values prior to serialization.
//   [  -> &open;
//   ]  -> &close;
const std::string Task::encode (const std::string& value) const
{
  std::string modified = value;

  str_replace (modified, "[",  "&open;");
  str_replace (modified, "]",  "&close;");

  return modified;
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   "  <- &dquot;
//   '  <- &squot; or &quot;
//   ,  <- &comma;
//   [  <- &open;
//   ]  <- &close;
//   :  <- &colon;
const std::string Task::decode (const std::string& value) const
{
  if (value.find ('&') != std::string::npos)
  {
    std::string modified = value;

    // Supported encodings.
    str_replace (modified, "&open;",  "[");
    str_replace (modified, "&close;", "]");

    // Support for deprecated encodings.  These cannot be removed or old files
    // will not be parsable.  Not just old files - completed.data can contain
    // tasks formatted/encoded using these.
    str_replace (modified, "&dquot;", "\"");
    str_replace (modified, "&quot;",  "'");
    str_replace (modified, "&squot;", "'");  // Deprecated 2.0
    str_replace (modified, "&comma;", ",");  // Deprecated 2.0
    str_replace (modified, "&colon;", ":");  // Deprecated 2.0

    return modified;
  }

  return value;
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
      if (var.first.substr (0, 13) == "urgency.user.")
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
          auto dot = uda.find (".");
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

  if (is_blocking && context.config.getBoolean ("urgency.inherit"))
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
  // Calling dependencyGetBlocked is rather expensive.
  // It is called recursively for each dependency in the chain here.
  std::vector <Task> blocked;
  dependencyGetBlocked (*this, blocked);

  float v = FLT_MIN;
  for (auto& task : blocked)
  {
    // Find highest urgency in all blocked tasks.
    v = std::max (v, task.urgency ());
  }

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
      get_date ("scheduled") < time (NULL))
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
    ISO8601d now;
    ISO8601d due (get_date ("due"));

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

  ISO8601d now;
  ISO8601d entry (get_date ("entry"));
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

////////////////////////////////////////////////////////////////////////////////
// Arguably does not belong here. This method reads the parse tree and calls
// Task methods. It could be a standalone function with no loss in access, as
// well as reducing the object depdendencies of Task.
//
// It came from the Command base object, but doesn't really belong there either.
void Task::modify (modType type, bool text_required /* = false */)
{
  context.debug ("Task::modify");
  std::string label = "  [1;37;43mMODIFICATION[0m ";

  std::string text = "";
  int modCount = 0;
  for (auto& a : context.cli2._args)
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
          // ::composeF4 will skip if the value is blank, but the presence of
          // the attribute will prevent ::validate from applying defaults.
          set (name, "");
          context.debug (label + name + " <-- ''");
          ++modCount;
        }
        else
        {
          Lexer::dequote (value);

          // Get the column info. Some columns are not modifiable.
          Column* column = context.columns[name];
          if (! column ||
              ! column->modifiable ())
            throw format (STRING_INVALID_MOD, name, value);

          // String type attributes only get DOM resolution, no eval.
          Variant evaluatedValue;
          if (name != "project" || Lexer::isDOM (value))
          {
            // Try to evaluate 'value'.  It might work.
            try
            {
              Eval e;
              e.addSource (domSource);
              e.addSource (namedDates);
              contextTask = *this;
              e.evaluateInfixExpression (value, evaluatedValue);
            }

            // Ah, fuck it.
            catch (...)
            {
              evaluatedValue = Variant (value);
            }
          }
          else
          {
            evaluatedValue = Variant (value);
          }

          // Dependencies are specified as IDs.
          if (name == "depends")
          {
            // Parse IDs
            std::vector <std::string> deps;
            split (deps, value, ',');

            // Apply or remove dendencies in turn.
            for (auto& dep : deps)
            {
              if (dep[0] == '-')
              {
                if (dep.length () == 37)
                  removeDependency (context.tdb2.pending.id (dep.substr (1)));
                else
                  removeDependency (strtol (dep.substr (1).c_str (), NULL, 10));
              }
              else
              {
                if (dep.length () == 36)
                  addDependency (context.tdb2.pending.id (dep));
                else
                  addDependency (strtol (dep.c_str (), NULL, 10));
              }
            }

            ++modCount;
          }

          // For those using the "tags:..." attribute directly.
          else if (name == "tags")
          {
            // TW-1701
            set ("tags", "");

            std::vector <std::string> tags;
            split (tags, value, ',');

            for (auto& tag : tags)
            {
              // If it's a DOM ref, eval it first.
              Lexer lexer (tag);
              std::string domRef;
              Lexer::Type type;
              if (lexer.token (domRef, type) &&
                  type == Lexer::Type::dom)
              {
                Eval e;
                e.addSource (domSource);
                e.addSource (namedDates);
                contextTask = *this;

                Variant v;
                e.evaluateInfixExpression (value, v);
                addTag ((std::string) v);
                context.debug (label + "tags <-- '" + (std::string) v + "' <-- '" + tag + "'");
              }
              else
              {
                addTag (tag);
                context.debug (label + "tags <-- '" + tag + "'");
              }

              feedback_special_tags (*this, tag);
            }

            ++modCount;
          }

          // Dates are special, maybe.
          else if (column->type () == "date")
          {
            // If v is duration, add 'now' to it, else store as date.
            if (evaluatedValue.type () == Variant::type_duration)
            {
              context.debug (label + name + " <-- '" + format ("{1}", format (evaluatedValue.get_duration ())) + "' <-- '" + (std::string) evaluatedValue + "' <-- '" + value + "'");
              Variant now;
              if (namedDates ("now", now))
                evaluatedValue += now;
            }
            else
            {
              evaluatedValue.cast (Variant::type_date);
              context.debug (label + name + " <-- '" + format ("{1}", evaluatedValue.get_date ()) + "' <-- '" + (std::string) evaluatedValue + "' <-- '" + value + "'");
            }

            // If a date doesn't parse (2/29/2014) then it evaluates to zero.
            if (value != "" &&
                evaluatedValue.get_date () == 0)
              throw format (STRING_DATE_INVALID_FORMAT, value, Variant::dateFormat);

            set (name, evaluatedValue.get_date ());
            ++modCount;
          }

          // Special case: type duration.
          // Note: "recur" is marked as type "string" to force storage in raw form.
          else if (name == "recur")
          {
            // The duration is stored in raw form, but it must still be valid,
            // and therefore is parsed first.

            if (evaluatedValue.type () == Variant::type_duration)
            {
              // Store the raw value, for 'recur'.
              context.debug (label + name + " <-- '" + value + "'");
              set (name, value);
              ++modCount;
            }
            else
              throw format (STRING_TASK_INVALID_DUR, value);
          }

          else if (column->type () == "duration")
          {
            // The duration is stored in raw form, but it must still be valid,
            // and therefore is parsed first.

            if (evaluatedValue.type () == Variant::type_duration)
            {
              // Store the raw value, for 'recur'.
              context.debug (label + name + " <-- " + (std::string) evaluatedValue + " <-- '" + value + "'");
              set (name, evaluatedValue);
              ++modCount;
            }
            else
              throw format (STRING_TASK_INVALID_DUR, value);
          }

          // Need handling for numeric types, used by UDAs.
          else if (column->type () == "numeric")
          {
            context.debug (label + name + " <-- '" + evaluatedValue.get_string () + "' <-- '" + value + "'");

            // If the result is not readily convertible to a numeric value,
            // then this is an error.
            if (evaluatedValue.type () == Variant::type_string)
              throw format (STRING_UDA_NUMERIC, evaluatedValue.get_string ());

            set (name, evaluatedValue);
            ++modCount;
          }

          // String type columns are not eval'd.  Welll, not much.
          else if (column->type () == "string")
          {
            std::string strValue = (std::string) evaluatedValue;
            if (column->validate (strValue))
            {
              if (column->can_modify ())
              {
                std::string col_value = column->modify (strValue);
                context.debug (label + name + " <-- '" + col_value + "' <-- '" + strValue + "' <-- '" + value + "'");
                (*this).set (name, col_value);
              }
              else
              {
                context.debug (label + name + " <-- '" + strValue + "' <-- '" + value + "'");
                (*this).set (name, strValue);
              }

              ++modCount;
            }
            else
              throw format (STRING_INVALID_MOD, name, value);
          }

          else
            throw format (STRING_TASK_INVALID_COL_TYPE, column->type (), name);
        }
      }

      // Perform description/annotation substitution.
      else if (a._lextype == Lexer::Type::substitution)
      {
        context.debug (label + "substitute " + a.attribute ("raw"));
        substitute (a.attribute ("from"),
                    a.attribute ("to"),
                    a.attribute ("flags"));
        ++modCount;
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
          context.debug (label + "tags <-- add '" + tag + "'");
          addTag (tag);
          feedback_special_tags (*this, tag);
        }
        else
        {
          context.debug (label + "tags <-- remove '" + tag + "'");
          removeTag (tag);
        }

        ++modCount;
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
      context.debug (label + "description <-- '" + text + "'");
      set ("description", text);
      break;

    case modPrepend:
      context.debug (label + "description <-- '" + text + "' + description");
      set ("description", text + " " + get ("description"));
      break;

    case modAppend:
      context.debug (label + "description <-- description + '" + text + "'");
      set ("description", get ("description") + " " + text);
      break;

    case modAnnotate:
      context.debug (label + "new annotation <-- '" + text + "'");
      addAnnotation (text);
      break;
    }
  }
  else if (modCount == 0 && text_required)
  {
    throw std::string (STRING_CMD_MODIFY_NEED_TEXT);
  }
}

////////////////////////////////////////////////////////////////////////////////
