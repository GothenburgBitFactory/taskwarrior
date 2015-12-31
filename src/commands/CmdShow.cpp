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
#include <CmdShow.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <text.h>
#include <i18n.h>
#include <main.h>
#include <Context.h>
#include <FS.h>
#include <ViewText.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdShow::CmdShow ()
{
  _keyword               = "show";
  _usage                 = "task          show [all | substring]";
  _description           = STRING_CMD_SHOW;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::config;
}

////////////////////////////////////////////////////////////////////////////////
int CmdShow::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  // Obtain the arguments from the description.  That way, things like '--'
  // have already been handled.
  std::vector <std::string> words = context.cli2.getWords ();
  if (words.size () > 1)
    throw std::string (STRING_CMD_SHOW_ARGS);

  int width = context.getWidth ();

  // Complain about configuration variables that are not recognized.
  // These are the regular configuration variables.
  // Note that there is a leading and trailing space, to make it easier to
  // search for whole words.
  std::string recognized =
    " abbreviation.minimum"
    " active.indicator"
    " allow.empty.filter"
    " avoidlastcolumn"
    " bulk"
    " calendar.details"
    " calendar.details.report"
    " calendar.holidays"
    " calendar.legend"
    " calendar.offset"
    " calendar.offset.value"
    " color"
    " color.active"
    " color.alternate"
    " color.blocked"
    " color.blocking"
    " color.burndown.done"
    " color.burndown.pending"
    " color.burndown.started"
    " color.calendar.due"
    " color.calendar.due.today"
    " color.calendar.holiday"
    " color.calendar.overdue"
    " color.calendar.today"
    " color.calendar.weekend"
    " color.calendar.weeknumber"
    " color.completed"
    " color.debug"
    " color.deleted"
    " color.due"
    " color.due.today"
    " color.warning"
    " color.error"
    " color.footnote"
    " color.header"
    " color.history.add"
    " color.history.delete"
    " color.history.done"
    " color.label"
    " color.label.sort"
    " color.overdue"
    " color.recurring"
    " color.scheduled"
    " color.summary.background"
    " color.summary.bar"
    " color.sync.added"
    " color.sync.changed"
    " color.sync.rejected"
    " color.tagged"
    " color.undo.after"
    " color.undo.before"
    " color.until"
    " column.padding"
    " complete.all.tags"
    " confirmation"
    " context"
    " data.location"
    " dateformat"
    " dateformat.annotation"
    " dateformat.edit"
    " dateformat.holiday"
    " dateformat.info"
    " dateformat.report"
    " date.iso"
    " debug"
    " debug.hooks"
    " debug.parser"
    " debug.tls"
    " default.command"
    " default.due"
    " default.project"
    " defaultheight"
    " defaultwidth"
    " dependency.confirmation"
    " dependency.indicator"
    " dependency.reminder"
    " detection"
    " displayweeknumber"
    " due"
    " editor"
    " exit.on.missing.db"
    " expressions"
    " fontunderline"
    " gc"
    " hooks"
    " hyphenate"
    " indent.annotation"
    " indent.report"
    " journal.info"
    " journal.time"
    " journal.time.start.annotation"
    " journal.time.stop.annotation"
    " json.array"
    " json.depends.array"
    " list.all.projects"
    " list.all.tags"
    " locking"
    " monthsperline"
    " nag"
    " obfuscate"
    " print.empty.columns"
    " recurrence"
    " recurrence.confirmation"
    " recurrence.indicator"
    " recurrence.limit"
    " regex"
    " reserved.lines"
    " row.padding"
    " rule.color.merge"
    " rule.precedence.color"
    " search.case.sensitive"
    " sugar"
    " summary.all.projects"
    " tag.indicator"
    " taskd.server"
    " taskd.ca"
    " taskd.certificate"
    " taskd.ciphers"
    " taskd.credentials"
    " taskd.key"
    " taskd.trust"
    " undo.style"
    " urgency.active.coefficient"
    " urgency.scheduled.coefficient"
    " urgency.annotations.coefficient"
    " urgency.blocked.coefficient"
    " urgency.blocking.coefficient"
    " urgency.due.coefficient"
    " urgency.project.coefficient"
    " urgency.tags.coefficient"
    " urgency.waiting.coefficient"
    " urgency.age.coefficient"
    " urgency.age.max"
    " urgency.inherit"
    " verbose"
    " weekstart"
    " xterm.title"
    " ";

  // This configuration variable is supported, but not documented.  It exists
  // so that unit tests can force color to be on even when the output from task
  // is redirected to a file, or stdout is not a tty.
  recognized += "_forcecolor ";

  std::vector <std::string> unrecognized;
  for (auto& i : context.config)
  {
    // Disallow partial matches by tacking a leading and trailing space on each
    // variable name.
    std::string pattern = " " + i.first + " ";
    if (recognized.find (pattern) == std::string::npos)
    {
      // These are special configuration variables, because their name is
      // dynamic.
      if (i.first.substr (0, 14) != "color.keyword."        &&
          i.first.substr (0, 14) != "color.project."        &&
          i.first.substr (0, 10) != "color.tag."            &&
          i.first.substr (0, 10) != "color.uda."            &&
          i.first.substr (0,  8) != "context."              &&
          i.first.substr (0,  8) != "holiday."              &&
          i.first.substr (0,  7) != "report."               &&
          i.first.substr (0,  6) != "alias."                &&
          i.first.substr (0,  5) != "hook."                 &&
          i.first.substr (0,  4) != "uda."                  &&
          i.first.substr (0,  8) != "default."              &&
          i.first.substr (0, 21) != "urgency.user.project." &&
          i.first.substr (0, 17) != "urgency.user.tag."     &&
          i.first.substr (0, 21) != "urgency.user.keyword." &&
          i.first.substr (0, 12) != "urgency.uda.")
      {
        unrecognized.push_back (i.first);
      }
    }
  }

  // Find all the values that match the defaults, for highlighting.
  std::vector <std::string> default_values;
  Config default_config;
  default_config.setDefaults ();

  for (auto& i : context.config)
    if (i.second != default_config.get (i.first))
      default_values.push_back (i.first);

  // Create output view.
  ViewText view;
  view.width (width);
  view.add (Column::factory ("string", STRING_CMD_SHOW_CONF_VAR));
  view.add (Column::factory ("string", STRING_CMD_SHOW_CONF_VALUE));

  Color error;
  Color warning;
  if (context.color ())
  {
    error   = Color (context.config.get ("color.error"));
    warning = Color (context.config.get ("color.warning"));

    Color label (context.config.get ("color.label"));
    view.colorHeader (label);
  }

  bool issue_error = false;
  bool issue_warning = false;

  std::string section;

  // Look for the first plausible argument which could be a pattern 
  if (words.size ())
    section = words[0];

  if (section == "all")
    section = "";

  std::string::size_type loc;
  for (auto& i : context.config)
  {
    loc = i.first.find (section, 0);
    if (loc != std::string::npos)
    {
      // Look for unrecognized.
      Color color;
      if (std::find (unrecognized.begin (), unrecognized.end (), i.first) != unrecognized.end ())
      {
        issue_error = true;
        color = error;
      }
      else if (std::find (default_values.begin (), default_values.end (), i.first) != default_values.end ())
      {
        issue_warning = true;
        color = warning;
      }

      std::string value = i.second;
      int row = view.addRow ();
      view.set (row, 0, i.first, color);
      view.set (row, 1, value, color);

      if (default_config[i.first] != value &&
          default_config[i.first] != "")
      {
        row = view.addRow ();
        view.set (row, 0, std::string ("  ") + STRING_CMD_SHOW_CONF_DEFAULT, color);
        view.set (row, 1, default_config[i.first], color);
      }
    }
  }

  out << "\n"
      << view.render ()
      << (view.rows () == 0 ? STRING_CMD_SHOW_NONE : "")
      << (view.rows () == 0 ? "\n\n" : "\n");

  if (issue_warning)
  {
    out << STRING_CMD_SHOW_DIFFER << "\n";

    if (context.color () && warning.nontrivial ())
      out << "  "
          << format (STRING_CMD_SHOW_DIFFER_COLOR, warning.colorize ("color"))
          << "\n\n";
  }

  // Display the unrecognized variables.
  if (issue_error)
  {
    out << STRING_CMD_SHOW_UNREC << "\n";

    for (auto& i : unrecognized)
      out << "  " << i << "\n";

    if (context.color () && error.nontrivial ())
      out << "\n" << format (STRING_CMD_SHOW_DIFFER_COLOR, error.colorize ("color"));

    out << "\n\n";
  }

  out << legacyCheckForDeprecatedVariables ();
  out << legacyCheckForDeprecatedColumns ();

  // TODO Check for referenced but missing theme files.
  // TODO Check for referenced but missing string files.

  // Check for bad values in rc.calendar.details.
  std::string calendardetails = context.config.get ("calendar.details");
  if (calendardetails != "full"   &&
      calendardetails != "sparse" &&
      calendardetails != "none")
    out << format (STRING_CMD_SHOW_CONFIG_ERROR, "calendar.details", calendardetails)
        << "\n";

  // Check for bad values in rc.calendar.holidays.
  std::string calendarholidays = context.config.get ("calendar.holidays");
  if (calendarholidays != "full"   &&
      calendarholidays != "sparse" &&
      calendarholidays != "none")
    out << format (STRING_CMD_SHOW_CONFIG_ERROR, "calendar.holidays", calendarholidays)
        << "\n";

  // Verify installation.  This is mentioned in the documentation as the way
  // to ensure everything is properly installed.

  if (context.config.size () == 0)
  {
    out << STRING_CMD_SHOW_EMPTY << "\n";
    rc = 1;
  }
  else
  {
    Directory location (context.config.get ("data.location"));

    if (location._data == "")
      out << STRING_CMD_SHOW_NO_LOCATION << "\n";

    if (! location.exists ())
      out << STRING_CMD_SHOW_LOC_EXIST << "\n";
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdShowRaw::CmdShowRaw ()
{
  _keyword     = "_show";
  _usage       = "task          _show";
  _description = STRING_CMD_SHOWRAW;
  _read_only   = true;
  _displays_id = false;
  _category    = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdShowRaw::execute (std::string& output)
{
  // Get all the settings.
  std::vector <std::string> all;
  context.config.all (all);

  // Sort alphabetically by name.
  std::sort (all.begin (), all.end ());

  // Display them all.
  std::stringstream out;
  for (auto& i : all)
    out << i << '=' << context.config.get (i) << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
