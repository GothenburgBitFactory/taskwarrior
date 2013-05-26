////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

#include <stdlib.h>
#include <Context.h>
#include <Date.h>
#include <text.h>
#include <util.h>
#include <main.h>

extern Context context;

static std::map <std::string, Color> gsColor;
static std::vector <std::string> gsPrecedence;
static Date now;

////////////////////////////////////////////////////////////////////////////////
void initializeColorRules ()
{
  gsColor.clear ();
  gsPrecedence.clear ();

  // Load all the configuration values, filter to only the ones that begin with
  // "color.", then store name/value in gsColor, and name in rules.
  std::vector <std::string> rules;
  Config::const_iterator v;
  for (v = context.config.begin (); v != context.config.end (); ++v)
  {
    if (v->first.substr (0, 6) == "color.")
    {
      Color c (v->second);
      gsColor[v->first] = c;

      rules.push_back (v->first);
    }
  }

  // Load the rule.precedence.color list, split it, then autocomplete against
  // the 'rules' vector loaded above.
  std::vector <std::string> results;
  std::vector <std::string> precedence;
  split (precedence, context.config.get ("rule.precedence.color"), ',');

  std::vector <std::string>::iterator p;
  for (p = precedence.begin (); p != precedence.end (); ++p)
  {
    // Add the leading "color." string.
    std::string rule = "color." + *p;
    autoComplete (rule, rules, results, 3); // Hard-coded 3.

    std::vector <std::string>::iterator r;
    for (r = results.begin (); r != results.end (); ++r)
      gsPrecedence.push_back (*r);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeBlocked (Task& task, const Color& base, Color& c)
{
  if (task.is_blocked)
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeBlocking (Task& task, const Color& base, Color& c)
{
  if (task.is_blocking)
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeTagged (Task& task, const Color& base, Color& c)
{
  if (task.getTagCount ())
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizePriorityL (Task& task, const Color& base, Color& c)
{
  if (task.get ("priority") == "L")
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizePriorityM (Task& task, const Color& base, Color& c)
{
  if (task.get ("priority") == "M")
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizePriorityH (Task& task, const Color& base, Color& c)
{
  if (task.get ("priority") == "H")
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizePriorityNone (Task& task, const Color& base, Color& c)
{
  if (task.get ("priority") == "")
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeActive (Task& task, const Color& base, Color& c)
{
  if (task.has ("start") &&
      !task.has ("end"))
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeScheduled (Task& task, const Color& base, Color& c)
{
  if (task.has ("scheduled") &&
      Date (task.get_date ("scheduled")) <= now)
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeTag (Task& task, const std::string& rule, const Color& base, Color& c)
{
  if (task.hasTag (rule.substr (10)))
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeProject (Task& task, const std::string& rule, const Color& base, Color& c)
{
  // Observe the case sensitivity setting.
  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  std::string project = task.get ("project");
  std::string rule_trunc = rule.substr (14);

  // Match project names leftmost.
  if (rule_trunc.length () <= project.length ())
    if (compare (rule_trunc, project.substr (0, rule_trunc.length ()), sensitive))
      c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeProjectNone (Task& task, const Color& base, Color& c)
{
  if (task.get ("project") == "")
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeTagNone (Task& task, const Color& base, Color& c)
{
  if (task.getTagCount () == 0)
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeKeyword (Task& task, const std::string& rule, const Color& base, Color& c)
{
  // Observe the case sensitivity setting.
  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  // The easiest thing to check is the description, because it is just one
  // attribute.
  if (find (task.get ("description"), rule.substr (14), sensitive) != std::string::npos)
    c.blend (base);

  // Failing the description check, look at all annotations, returning on the
  // first match.
  else
  {
    Task::iterator it;
    for (it = task.begin (); it != task.end (); ++it)
    {
      if (it->first.substr (0, 11) == "annotation_" &&
          find (it->second, rule.substr (14), sensitive) != std::string::npos)
      {
        c.blend (base);
        return;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeUDA (Task& task, const std::string& rule, const Color& base, Color& c)
{
  if (task.has (rule.substr (10)))
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeDue (Task& task, const Color& base, Color& c)
{
  if (task.has ("due"))
  {
    Task::status status = task.getStatus ();
    if (status != Task::completed &&
        status != Task::deleted   &&
        getDueState (task.get ("due")) == 1)
      c.blend (base);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeDueToday (Task& task, const Color& base, Color& c)
{
  if (task.has ("due"))
  {
    Task::status status = task.getStatus ();
    if (status != Task::completed &&
        status != Task::deleted   &&
        getDueState (task.get ("due")) == 2)
      c.blend (base);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeOverdue (Task& task, const Color& base, Color& c)
{
  if (task.has ("due"))
  {
    Task::status status = task.getStatus ();
    if (status != Task::completed &&
        status != Task::deleted   &&
        getDueState (task.get ("due")) == 3)
      c.blend (base);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeRecurring (Task& task, const Color& base, Color& c)
{
  if (task.has ("recur"))
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeCompleted (Task& task, const Color& base, Color& c)
{
  if (task.getStatus () == Task::completed)
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeDeleted (Task& task, const Color& base, Color& c)
{
  if (task.getStatus () == Task::deleted)
    c.blend (base);
}

////////////////////////////////////////////////////////////////////////////////
void autoColorize (Task& task, Color& c)
{
  // The special tag 'nocolor' overrides all auto and specific colorization.
  if (!context.color () ||
      task.hasTag ("nocolor"))
  {
    c = Color ();
    return;
  }

  // Note: c already contains colors specifically assigned via command.
  // Note: These rules form a hierarchy - the last rule is King, hence the
  //       reverse iterator.
  std::vector <std::string>::reverse_iterator r;
  for (r = gsPrecedence.rbegin (); r != gsPrecedence.rend (); ++r)
  {
    Color base = gsColor[*r];
    if (base.nontrivial ())
    {
           if (*r == "color.blocked")                 colorizeBlocked      (task, base, c);
      else if (*r == "color.blocking")                colorizeBlocking     (task, base, c);
      else if (*r == "color.tagged")                  colorizeTagged       (task, base, c);
      else if (*r == "color.pri.L")                   colorizePriorityL    (task, base, c);
      else if (*r == "color.pri.M")                   colorizePriorityM    (task, base, c);
      else if (*r == "color.pri.H")                   colorizePriorityH    (task, base, c);
      else if (*r == "color.pri.none")                colorizePriorityNone (task, base, c);
      else if (*r == "color.active")                  colorizeActive       (task, base, c);
      else if (*r == "color.scheduled")               colorizeScheduled    (task, base, c);
      else if (*r == "color.project.none")            colorizeProjectNone  (task, base, c);
      else if (*r == "color.tag.none")                colorizeTagNone      (task, base, c);
      else if (*r == "color.due")                     colorizeDue          (task, base, c);
      else if (*r == "color.due.today")               colorizeDueToday     (task, base, c);
      else if (*r == "color.overdue")                 colorizeOverdue      (task, base, c);
      else if (*r == "color.recurring")               colorizeRecurring    (task, base, c);
      else if (*r == "color.completed")               colorizeCompleted    (task, base, c);
      else if (*r == "color.deleted")                 colorizeDeleted      (task, base, c);

      // Wildcards
      else if (r->substr (0, 10) == "color.tag.")     colorizeTag          (task, *r, base, c);
      else if (r->substr (0, 14) == "color.project.") colorizeProject      (task, *r, base, c);
      else if (r->substr (0, 14) == "color.keyword.") colorizeKeyword      (task, *r, base, c);
      else if (r->substr (0, 10) == "color.uda.")     colorizeUDA          (task, *r, base, c);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeHeader (const std::string& input)
{
  if (gsColor["color.header"].nontrivial ())
    return gsColor["color.header"].colorize (input);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeFootnote (const std::string& input)
{
  if (gsColor["color.footnote"].nontrivial ())
    return gsColor["color.footnote"].colorize (input);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeError (const std::string& input)
{
  if (gsColor["color.error"].nontrivial ())
    return gsColor["color.error"].colorize (input);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeDebug (const std::string& input)
{
  if (gsColor["color.debug"].nontrivial ())
    return gsColor["color.debug"].colorize (input);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
