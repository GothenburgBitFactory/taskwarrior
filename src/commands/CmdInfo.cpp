////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Paul Beckingham, Federico Hernandez.
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
#include <CmdInfo.h>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <Context.h>
#include <Filter.h>
#include <Datetime.h>
#include <Duration.h>
#include <main.h>
#include <shared.h>
#include <format.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
CmdInfo::CmdInfo ()
{
  _keyword               = "information";
  _usage                 = "task <filter> information";
  _description           = "Shows all data and metadata";
  _read_only             = true;

  // This is inaccurate, but it does prevent a GC.  While this doesn't make a
  // lot of sense, given that the info command shows the ID, it does mimic the
  // behavior of versions prior to 2.0, which the test suite relies upon.
  //
  // Once the test suite is completely modified, this can be corrected.
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdInfo::execute (std::string& output)
{
  auto rc = 0;

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  if (! filtered.size ())
  {
    Context::getContext ().footnote ("No matches.");
    rc = 1;
  }

  // Get the undo data.
  std::vector <std::string> undo;
  if (Context::getContext ().config.getBoolean ("journal.info"))
    undo = Context::getContext ().tdb2.undo.get_lines ();

  // Determine the output date format, which uses a hierarchy of definitions.
  //   rc.dateformat.info
  //   rc.dateformat
  auto dateformat = Context::getContext ().config.get ("dateformat.info");
  if (dateformat == "")
    dateformat = Context::getContext ().config.get ("dateformat");

  auto dateformatanno = Context::getContext ().config.get ("dateformat.annotation");
  if (dateformatanno == "")
    dateformatanno = dateformat;

  // Render each task.
  std::stringstream out;
  for (auto& task : filtered)
  {
    Table view;
    view.width (Context::getContext ().getWidth ());
    if (Context::getContext ().config.getBoolean ("obfuscate"))
      view.obfuscate ();
    if (Context::getContext ().color ())
      view.forceColor ();
    view.add ("Name");
    view.add ("Value");
    setHeaderUnderline (view);

    Datetime now;

    // id
    auto row = view.addRow ();
    view.set (row, 0, "ID");
    view.set (row, 1, (task.id ? format (task.id) : "-"));

    std::string status = Lexer::ucFirst (Task::statusToText (task.getStatus ()));

    // description
    Color c;
    autoColorize (task, c);
    auto description = task.get ("description");
    auto indent = Context::getContext ().config.getInteger ("indent.annotation");

    for (auto& anno : task.getAnnotations ())
      description += '\n'
                   + std::string (indent, ' ')
                   + Datetime (anno.first.substr (11)).toString (dateformatanno)
                   + ' '
                   + anno.second;

    row = view.addRow ();
    view.set (row, 0, "Description");
    view.set (row, 1, description, c);

    // status
    row = view.addRow ();
    view.set (row, 0, "Status");
    view.set (row, 1, status);

    // project
    if (task.has ("project"))
    {
      row = view.addRow ();
      view.set (row, 0, "Project");
      view.set (row, 1, task.get ("project"));
    }

    // dependencies: blocked
    {
      auto blocked = dependencyGetBlocking (task);
      if (blocked.size ())
      {
        std::stringstream message;
        for (auto& block : blocked)
          message << block.id << ' ' << block.get ("description") << '\n';

        row = view.addRow ();
        view.set (row, 0, "This task blocked by");
        view.set (row, 1, message.str ());
      }
    }

    // dependencies: blocking
    {
      auto blocking = dependencyGetBlocked (task);
      if (blocking.size ())
      {
        std::stringstream message;
        for (auto& block : blocking)
          message << block.id << ' ' << block.get ("description") << '\n';

        row = view.addRow ();
        view.set (row, 0, "This task is blocking");
        view.set (row, 1, message.str ());
      }
    }

    // recur
    if (task.has ("recur"))
    {
      row = view.addRow ();
      view.set (row, 0, "Recurrence");
      view.set (row, 1, task.get ("recur"));
    }

    // parent
    // 2017-01-07: Deprecated in 2.6.0
    if (task.has ("parent"))
    {
      row = view.addRow ();
      view.set (row, 0, "Parent task");
      view.set (row, 1, task.get ("parent"));
    }

    // mask
    // 2017-01-07: Deprecated in 2.6.0
    if (task.has ("mask"))
    {
      row = view.addRow ();
      view.set (row, 0, "Mask");
      view.set (row, 1, task.get ("mask"));
    }

    // imask
    // 2017-01-07: Deprecated in 2.6.0
    if (task.has ("imask"))
    {
      row = view.addRow ();
      view.set (row, 0, "Mask Index");
      view.set (row, 1, task.get ("imask"));
    }

    // template
    if (task.has ("template"))
    {
      row = view.addRow ();
      view.set (row, 0, "Template task");
      view.set (row, 1, task.get ("template"));
    }

    // last
    if (task.has ("last"))
    {
      row = view.addRow ();
      view.set (row, 0, "Last instance");
      view.set (row, 1, task.get ("last"));
    }

    // rtype
    if (task.has ("rtype"))
    {
      row = view.addRow ();
      view.set (row, 0, "Recurrence type");
      view.set (row, 1, task.get ("rtype"));
    }

    // entry
    row = view.addRow ();
    view.set (row, 0, "Entered");
    Datetime dt (task.get_date ("entry"));
    std::string entry = dt.toString (dateformat);

    std::string age;
    auto created = task.get ("entry");
    if (created.length ())
    {
      Datetime dt (strtol (created.c_str (), nullptr, 10));
      age = Duration (now - dt).formatVague ();
    }

    view.set (row, 1, entry + " (" + age + ')');

    // wait
    if (task.has ("wait"))
    {
      row = view.addRow ();
      view.set (row, 0, "Waiting until");
      view.set (row, 1, Datetime (task.get_date ("wait")).toString (dateformat));
    }

    // scheduled
    if (task.has ("scheduled"))
    {
      row = view.addRow ();
      view.set (row, 0, "Scheduled");
      view.set (row, 1, Datetime (task.get_date ("scheduled")).toString (dateformat));
    }

    // start
    if (task.has ("start"))
    {
      row = view.addRow ();
      view.set (row, 0, "Start");
      view.set (row, 1, Datetime (task.get_date ("start")).toString (dateformat));
    }

    // due (colored)
    if (task.has ("due"))
    {
      row = view.addRow ();
      view.set (row, 0, "Due");
      view.set (row, 1, Datetime (task.get_date ("due")).toString (dateformat));
    }

    // end
    if (task.has ("end"))
    {
      row = view.addRow ();
      view.set (row, 0, "End");
      view.set (row, 1, Datetime (task.get_date ("end")).toString (dateformat));
    }

    // until
    if (task.has ("until"))
    {
      row = view.addRow ();
      view.set (row, 0, "Until");
      view.set (row, 1, Datetime (task.get_date ("until")).toString (dateformat));
    }

    // modified
    if (task.has ("modified"))
    {
      row = view.addRow ();
      view.set (row, 0, "Last modified");

      Datetime mod (task.get_date ("modified"));
      std::string age = Duration (now - mod).formatVague ();
      view.set (row, 1, mod.toString (dateformat) + " (" + age + ')');
    }

    // tags ...
    auto tags = task.getTags ();
    if (tags.size ())
    {
      auto allTags = join (" ", tags);

      row = view.addRow ();
      view.set (row, 0, "Tags");
      view.set (row, 1, allTags);
    }

    // Virtual tags.
    {
      // Note: This list must match that in Task::hasTag.
      // Note: This list must match that in ::feedback_reserved_tags.
      std::string virtualTags = "";
      if (task.hasTag ("ACTIVE"))    virtualTags += "ACTIVE ";
      if (task.hasTag ("ANNOTATED")) virtualTags += "ANNOTATED ";
      if (task.hasTag ("BLOCKED"))   virtualTags += "BLOCKED ";
      if (task.hasTag ("BLOCKING"))  virtualTags += "BLOCKING ";
      if (task.hasTag ("CHILD"))     virtualTags += "CHILD ";          // 2017-01-07: Deprecated in 2.6.0
      if (task.hasTag ("COMPLETED")) virtualTags += "COMPLETED ";
      if (task.hasTag ("DELETED"))   virtualTags += "DELETED ";
      if (task.hasTag ("DUE"))       virtualTags += "DUE ";
      if (task.hasTag ("DUETODAY"))  virtualTags += "DUETODAY ";      // 2016-03-29: Deprecated in 2.6.0
      if (task.hasTag ("INSTANCE"))  virtualTags += "INSTANCE ";
      if (task.hasTag ("LATEST"))    virtualTags += "LATEST ";
      if (task.hasTag ("MONTH"))     virtualTags += "MONTH ";
      if (task.hasTag ("ORPHAN"))    virtualTags += "ORPHAN ";
      if (task.hasTag ("OVERDUE"))   virtualTags += "OVERDUE ";
      if (task.hasTag ("PARENT"))    virtualTags += "PARENT ";         // 2017-01-07: Deprecated in 2.6.0
      if (task.hasTag ("PENDING"))   virtualTags += "PENDING ";
      if (task.hasTag ("PRIORITY"))  virtualTags += "PRIORITY ";
      if (task.hasTag ("PROJECT"))   virtualTags += "PROJECT";
      if (task.hasTag ("QUARTER"))   virtualTags += "QUARTER";
      if (task.hasTag ("READY"))     virtualTags += "READY ";
      if (task.hasTag ("SCHEDULED")) virtualTags += "SCHEDULED ";
      if (task.hasTag ("TAGGED"))    virtualTags += "TAGGED ";
      if (task.hasTag ("TEMPLATE"))  virtualTags += "TEMPLATE ";
      if (task.hasTag ("TODAY"))     virtualTags += "TODAY ";
      if (task.hasTag ("TOMORROW"))  virtualTags += "TOMORROW ";
      if (task.hasTag ("UDA"))       virtualTags += "UDA ";
      if (task.hasTag ("UNBLOCKED")) virtualTags += "UNBLOCKED ";
      if (task.hasTag ("UNTIL"))     virtualTags += "UNTIL ";
      if (task.hasTag ("WAITING"))   virtualTags += "WAITING ";
      if (task.hasTag ("WEEK"))      virtualTags += "WEEK ";
      if (task.hasTag ("YEAR"))      virtualTags += "YEAR ";
      if (task.hasTag ("YESTERDAY")) virtualTags += "YESTERDAY ";
      // If you update the above list, update src/commands/CmdInfo.cpp and src/commands/CmdTags.cpp as well.

      row = view.addRow ();
      view.set (row, 0, "Virtual tags");
      view.set (row, 1, virtualTags);
    }

    // uuid
    row = view.addRow ();
    view.set (row, 0, "UUID");
    auto uuid = task.get ("uuid");
    view.set (row, 1, uuid);

    // Task::urgency
    row = view.addRow ();
    view.set (row, 0, "Urgency");
    view.set (row, 1, format (task.urgency (), 4, 4));

    // Show any UDAs
    auto all = task.all ();
    for (auto& att: all)
    {
      if (Context::getContext ().columns.find (att) != Context::getContext ().columns.end ())
      {
        Column* col = Context::getContext ().columns[att];
        if (col->is_uda ())
        {
          auto value = task.get (att);
          if (value != "")
          {
            row = view.addRow ();
            view.set (row, 0, col->label ());

            if (col->type () == "date")
              value = Datetime (value).toString (dateformat);
            else if (col->type () == "duration")
            {
              Duration iso;
              std::string::size_type cursor = 0;
              if (iso.parse (value, cursor))
                value = (std::string) Variant (iso.toTime_t (), Variant::type_duration);
              else
                value = "PT0S";
            }

            view.set (row, 1, value);
          }
        }
      }
    }

    // Show any orphaned UDAs, which are identified by not being represented in
    // the context.columns map.
    for (auto& att : all)
    {
      if (att.substr (0, 11) != "annotation_" &&
          Context::getContext ().columns.find (att) == Context::getContext ().columns.end ())
      {
         row = view.addRow ();
         view.set (row, 0, '[' + att);
         view.set (row, 1, task.get (att) + ']');
      }
    }

    // Create a second table, containing urgency details, if necessary.
    Table urgencyDetails;
    if (task.urgency () != 0.0)
    {
      setHeaderUnderline (urgencyDetails);
      if (Context::getContext ().color ())
      {
        Color alternate (Context::getContext ().config.get ("color.alternate"));
        urgencyDetails.colorOdd (alternate);
        urgencyDetails.intraColorOdd (alternate);
      }

      if (Context::getContext ().config.getBoolean ("obfuscate"))
        urgencyDetails.obfuscate ();
      if (Context::getContext ().config.getBoolean ("color"))
        view.forceColor ();

      urgencyDetails.width (Context::getContext ().getWidth ());
      urgencyDetails.add (""); // Attribute
      urgencyDetails.add (""); // Value
      urgencyDetails.add (""); // *
      urgencyDetails.add (""); // Coefficient
      urgencyDetails.add (""); // =
      urgencyDetails.add (""); // Result

      urgencyTerm (urgencyDetails, "project",     task.urgency_project (),     Task::urgencyProjectCoefficient);
      urgencyTerm (urgencyDetails, "active",      task.urgency_active (),      Task::urgencyActiveCoefficient);
      urgencyTerm (urgencyDetails, "scheduled",   task.urgency_scheduled (),   Task::urgencyScheduledCoefficient);
      urgencyTerm (urgencyDetails, "waiting",     task.urgency_waiting (),     Task::urgencyWaitingCoefficient);
      urgencyTerm (urgencyDetails, "blocked",     task.urgency_blocked (),     Task::urgencyBlockedCoefficient);
      urgencyTerm (urgencyDetails, "blocking",    task.urgency_blocking (),    Task::urgencyBlockingCoefficient);
      urgencyTerm (urgencyDetails, "annotations", task.urgency_annotations (), Task::urgencyAnnotationsCoefficient);
      urgencyTerm (urgencyDetails, "tags",        task.urgency_tags (),        Task::urgencyTagsCoefficient);
      urgencyTerm (urgencyDetails, "due",         task.urgency_due (),         Task::urgencyDueCoefficient);
      urgencyTerm (urgencyDetails, "age",         task.urgency_age (),         Task::urgencyAgeCoefficient);

      // Tag, Project- and UDA-specific coefficients.
      for (auto& var : Task::coefficients)
      {
        if (var.first.substr (0, 13) == "urgency.user.")
        {
          // urgency.user.project.<project>.coefficient
          auto end = std::string::npos;
          if (var.first.substr (13, 8) == "project." &&
              (end = var.first.find (".coefficient")) != std::string::npos)
          {
            auto project = var.first.substr (21, end - 21);
            if (task.get ("project").find (project) == 0)
              urgencyTerm (urgencyDetails, "PROJECT " + project, 1.0, var.second);
          }

          // urgency.user.tag.<tag>.coefficient
          if (var.first.substr (13, 4) == "tag." &&
              (end = var.first.find (".coefficient")) != std::string::npos)
          {
            auto name = var.first.substr (17, end - 17);
            if (task.hasTag (name))
              urgencyTerm (urgencyDetails, "TAG " + name, 1.0, var.second);
          }

          // urgency.user.keyword.<keyword>.coefficient
          if (var.first.substr (13, 8) == "keyword." &&
              (end = var.first.find (".coefficient")) != std::string::npos)
          {
            auto keyword = var.first.substr (21, end - 21);
            if (task.get ("description").find (keyword) != std::string::npos)
              urgencyTerm (urgencyDetails, "KEYWORD " + keyword, 1.0, var.second);
          }
        }

        // urgency.uda.<name>.coefficient
        else if (var.first.substr (0, 12) == "urgency.uda.")
        {
          // urgency.uda.<name>.coefficient
          // urgency.uda.<name>.<value>.coefficient
          auto end = var.first.find (".coefficient");
          if (end != std::string::npos)
          {
            auto uda = var.first.substr (12, end - 12);
            auto dot = uda.find ('.');
            if (dot == std::string::npos)
            {
              // urgency.uda.<name>.coefficient
              if (task.has (uda))
                urgencyTerm (urgencyDetails, std::string ("UDA ") + uda, 1.0, var.second);
            }
            else
            {
              // urgency.uda.<name>.<value>.coefficient
              if (task.get (uda.substr(0, dot)) == uda.substr(dot+1))
                urgencyTerm (urgencyDetails, std::string ("UDA ") + uda, 1.0, var.second);
            }
          }
        }
      }

      row = urgencyDetails.addRow ();
      urgencyDetails.set (row, 5, rightJustify ("------", 6));
      row = urgencyDetails.addRow ();
      urgencyDetails.set (row, 5, rightJustify (format (task.urgency (), 4, 4), 6));
    }

    // Create a third table, containing undo log change details.
    Table journal;
    setHeaderUnderline (journal);

    if (Context::getContext ().config.getBoolean ("obfuscate"))
      journal.obfuscate ();
    if (Context::getContext ().config.getBoolean ("color"))
      journal.forceColor ();

    journal.width (Context::getContext ().getWidth ());
    journal.add ("Date");
    journal.add ("Modification");

    if (Context::getContext ().config.getBoolean ("journal.info") &&
        undo.size () > 3)
    {
      // Scan the undo data for entries matching this task, without making
      // copies.
      unsigned int i = 0;
      long last_timestamp = 0;
      while (i < undo.size ())
      {
        int when = i++;
        int previous = -1;

        if (! undo[i].compare (0, 3, "old", 3))
          previous = i++;

        int current = i++;
        i++; // Separator

        if (undo[current].find ("uuid:\"" + uuid) != std::string::npos)
        {
          if (previous != -1)
          {
            int row = journal.addRow ();

            Datetime timestamp (strtol (undo[when].substr (5).c_str (), nullptr, 10));
            journal.set (row, 0, timestamp.toString (dateformat));

            Task before (undo[previous].substr (4));
            Task after (undo[current].substr (4));
            journal.set (row, 1, taskInfoDifferences (before, after, dateformat, last_timestamp, Datetime(after.get("modified")).toEpoch()));
          }
        }
      }
    }

    out << optionalBlankLine ()
        << view.render ()
        << '\n';

    if (urgencyDetails.rows () > 0)
      out << urgencyDetails.render ()
          << '\n';

    if (journal.rows () > 0)
      out << journal.render ()
          << '\n';
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void CmdInfo::urgencyTerm (
  Table& view,
  const std::string& label,
  float measure,
  float coefficient) const
{
  auto value = measure * coefficient;
  if (value != 0.0)
  {
    auto row = view.addRow ();
    view.set (row, 0, "    " + label);
    view.set (row, 1, rightJustify (format (measure, 5, 3), 6));
    view.set (row, 2, "*");
    view.set (row, 3, rightJustify (format (coefficient, 4, 2), 4));
    view.set (row, 4, "=");
    view.set (row, 5, rightJustify (format (value, 5, 3), 6));
  }
}

////////////////////////////////////////////////////////////////////////////////
