////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <stdlib.h>
#include <Context.h>
#include <ISO8601.h>
#include <text.h>
#include <util.h>
#include <main.h>

extern Context context;

static std::map <std::string, Color> gsColor;
static std::vector <std::string> gsPrecedence;
static ISO8601d now;

////////////////////////////////////////////////////////////////////////////////
void initializeColorRules ()
{
  // If color is not enable/supported, short circuit.
  if (! context.color ())
    return;

  try
  {
    gsColor.clear ();
    gsPrecedence.clear ();

    // Load all the configuration values, filter to only the ones that begin with
    // "color.", then store name/value in gsColor, and name in rules.
    std::vector <std::string> rules;
    for (auto& v : context.config)
    {
      if (! v.first.compare (0, 6, "color.", 6))
      {
        Color c (v.second);
        gsColor[v.first] = c;

        rules.push_back (v.first);
      }
    }

    // Load the rule.precedence.color list, split it, then autocomplete against
    // the 'rules' vector loaded above.
    std::vector <std::string> results;
    std::vector <std::string> precedence;
    split (precedence, context.config.get ("rule.precedence.color"), ',');

    for (auto& p : precedence)
    {
      // Add the leading "color." string.
      std::string rule = "color." + p;
      autoComplete (rule, rules, results, 3); // Hard-coded 3.

      for (auto& r : results)
        gsPrecedence.push_back (r);
    }
  }

  catch (const std::string& e)
  {
    context.error (e);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void applyColor (const Color& base, Color& c, bool merge)
{
    if (merge)
      c.blend (base);
    else
      c = base;
}


////////////////////////////////////////////////////////////////////////////////
static void colorizeBlocked (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.is_blocked)
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeBlocking (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.is_blocking)
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeTagged (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.getTagCount ())
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeActive (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.has ("start") &&
      !task.has ("end"))
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeScheduled (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.has ("scheduled") &&
      ISO8601d (task.get_date ("scheduled")) <= now)
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeUntil (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.has ("until"))
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeTag (Task& task, const std::string& rule, const Color& base, Color& c, bool merge)
{
  if (task.hasTag (rule.substr (10)))
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeProject (Task& task, const std::string& rule, const Color& base, Color& c, bool merge)
{
  // Observe the case sensitivity setting.
  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  std::string project = task.get ("project");
  std::string rule_trunc = rule.substr (14);

  // Match project names leftmost.
  if (rule_trunc.length () <= project.length ())
    if (compare (rule_trunc, project.substr (0, rule_trunc.length ()), sensitive))
      applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeProjectNone (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.get ("project") == "")
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeTagNone (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.getTagCount () == 0)
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeKeyword (Task& task, const std::string& rule, const Color& base, Color& c, bool merge)
{
  // Observe the case sensitivity setting.
  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  // The easiest thing to check is the description, because it is just one
  // attribute.
  if (find (task.get ("description"), rule.substr (14), sensitive) != std::string::npos)
    applyColor (base, c, merge);

  // Failing the description check, look at all annotations, returning on the
  // first match.
  else
  {
    for (auto& it : task.data)
    {
      if (! it.first.compare (0, 11, "annotation_", 11) &&
          find (it.second, rule.substr (14), sensitive) != std::string::npos)
      {
        applyColor (base, c, merge);
        return;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeUDA (Task& task, const std::string& rule, const Color& base, Color& c, bool merge)
{
  // Is the rule color.uda.name.value or color.uda.name?
  size_t pos = rule.find (".", 10);
  if (pos == std::string::npos)
  {
    if (task.has (rule.substr (10)))
      applyColor (base, c, merge);
  }
  else
  {
    const std::string uda = rule.substr (10, pos - 10);
    const std::string val = rule.substr (pos + 1);
    if ((val == "none" && ! task.has (uda)) ||
        task.get (uda) == val)
      applyColor (base, c, merge);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeDue (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.has ("due"))
  {
    Task::status status = task.getStatus ();
    if (status != Task::completed &&
        status != Task::deleted   &&
        task.getDateState ("due") == Task::dateAfterToday)
      applyColor (base, c, merge);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeDueToday (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.has ("due"))
  {
    Task::status status = task.getStatus ();
    Task::dateState dateState = task.getDateState ("due");
    if (status != Task::completed &&
        status != Task::deleted   &&
        (dateState == Task::dateLaterToday || dateState == Task::dateEarlierToday))
      applyColor (base, c, merge);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeOverdue (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.has ("due"))
  {
    Task::status status = task.getStatus ();
    if (status != Task::completed &&
        status != Task::deleted   &&
        task.getDateState ("due") == Task::dateBeforeToday)
      applyColor (base, c, merge);
  }
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeRecurring (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.has ("recur"))
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeCompleted (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.getStatus () == Task::completed)
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
static void colorizeDeleted (Task& task, const Color& base, Color& c, bool merge)
{
  if (task.getStatus () == Task::deleted)
    applyColor (base, c, merge);
}

////////////////////////////////////////////////////////////////////////////////
void autoColorize (Task& task, Color& c)
{
  // The special tag 'nocolor' overrides all auto and specific colorization.
  if (! context.color () ||
      task.hasTag ("nocolor"))
  {
    c = Color ();
    return;
  }

  bool merge = context.config.getBoolean ("rule.color.merge");

  // Note: c already contains colors specifically assigned via command.
  // Note: These rules form a hierarchy - the last rule is King, hence the
  //       reverse iterator.

  for (auto r = gsPrecedence.rbegin (); r != gsPrecedence.rend (); ++r)
  {
    Color base = gsColor[*r];
    if (base.nontrivial ())
    {
           if (*r == "color.blocked")                      colorizeBlocked      (task, base, c, merge);
      else if (*r == "color.blocking")                     colorizeBlocking     (task, base, c, merge);
      else if (*r == "color.tagged")                       colorizeTagged       (task, base, c, merge);
      else if (*r == "color.active")                       colorizeActive       (task, base, c, merge);
      else if (*r == "color.scheduled")                    colorizeScheduled    (task, base, c, merge);
      else if (*r == "color.until")                        colorizeUntil        (task, base, c, merge);
      else if (*r == "color.project.none")                 colorizeProjectNone  (task, base, c, merge);
      else if (*r == "color.tag.none")                     colorizeTagNone      (task, base, c, merge);
      else if (*r == "color.due")                          colorizeDue          (task, base, c, merge);
      else if (*r == "color.due.today")                    colorizeDueToday     (task, base, c, merge);
      else if (*r == "color.overdue")                      colorizeOverdue      (task, base, c, merge);
      else if (*r == "color.recurring")                    colorizeRecurring    (task, base, c, merge);
      else if (*r == "color.completed")                    colorizeCompleted    (task, base, c, merge);
      else if (*r == "color.deleted")                      colorizeDeleted      (task, base, c, merge);

      // Wildcards
      else if (! r->compare (0, 10, "color.tag.", 10))     colorizeTag          (task, *r, base, c, merge);
      else if (! r->compare (0, 14, "color.project.", 14)) colorizeProject      (task, *r, base, c, merge);
      else if (! r->compare (0, 14, "color.keyword.", 14)) colorizeKeyword      (task, *r, base, c, merge);
      else if (! r->compare (0, 10, "color.uda.", 10))     colorizeUDA          (task, *r, base, c, merge);
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
