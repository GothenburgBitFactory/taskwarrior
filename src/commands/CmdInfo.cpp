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
#include <CmdInfo.h>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <Context.h>
#include <Filter.h>
#include <ISO8601.h>
#include <main.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdInfo::CmdInfo ()
{
  _keyword               = "information";
  _usage                 = "task <filter> information";
  _description           = STRING_CMD_INFO_USAGE;
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
  int rc = 0;

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  if (! filtered.size ())
  {
    context.footnote (STRING_FEEDBACK_NO_MATCH);
    rc = 1;
  }

  // Get the undo data.
  std::vector <std::string> undo;
  if (context.config.getBoolean ("journal.info"))
    undo = context.tdb2.undo.get_lines ();

  // Determine the output date format, which uses a hierarchy of definitions.
  //   rc.dateformat.info
  //   rc.dateformat
  std::string dateformat = context.config.get ("dateformat.info");
  if (dateformat == "")
    dateformat = context.config.get ("dateformat");

  std::string dateformatanno = context.config.get ("dateformat.annotation");
  if (dateformatanno == "")
    dateformatanno = dateformat;

  // Render each task.
  std::stringstream out;
  for (auto& task : filtered)
  {
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_NAME));
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_VALUE));

    // If an alternating row color is specified, notify the table.
    if (context.color ())
    {
      Color alternate (context.config.get ("color.alternate"));
      view.colorOdd (alternate);
      view.intraColorOdd (alternate);

      Color label (context.config.get ("color.label"));
      view.colorHeader (label);
    }

    ISO8601d now;

    // id
    int row = view.addRow ();
    view.set (row, 0, STRING_COLUMN_LABEL_ID);
    view.set (row, 1, (task.id ? format (task.id) : "-"));

    std::string status = Lexer::ucFirst (Task::statusToText (task.getStatus ()));

    // description
    Color c;
    autoColorize (task, c);
    std::string description = task.get ("description");
    int indent = context.config.getInteger ("indent.annotation");

    std::map <std::string, std::string> annotations;
    task.getAnnotations (annotations);
    for (auto& anno : annotations)
      description += "\n"
                   + std::string (indent, ' ')
                   + ISO8601d (anno.first.substr (11)).toString (dateformatanno)
                   + " "
                   + anno.second;

    row = view.addRow ();
    view.set (row, 0, STRING_COLUMN_LABEL_DESC);
    view.set (row, 1, description, c);

    // status
    row = view.addRow ();
    view.set (row, 0, STRING_COLUMN_LABEL_STATUS);
    view.set (row, 1, status);

    // project
    if (task.has ("project"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_PROJECT);
      view.set (row, 1, task.get ("project"));
    }

    // dependencies: blocked
    {
      std::vector <Task> blocked;
      dependencyGetBlocking (task, blocked);
      if (blocked.size ())
      {
        std::stringstream message;
        for (auto& block : blocked)
          message << block.id << " " << block.get ("description") << "\n";

        row = view.addRow ();
        view.set (row, 0, STRING_CMD_INFO_BLOCKED);
        view.set (row, 1, message.str ());
      }
    }

    // dependencies: blocking
    {
      std::vector <Task> blocking;
      dependencyGetBlocked (task, blocking);
      if (blocking.size ())
      {
        std::stringstream message;
        for (auto& block : blocking)
          message << block.id << " " << block.get ("description") << "\n";

        row = view.addRow ();
        view.set (row, 0, STRING_CMD_INFO_BLOCKING);
        view.set (row, 1, message.str ());
      }
    }

    // recur
    if (task.has ("recur"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_RECUR_L);
      view.set (row, 1, task.get ("recur"));
    }

    // parent
    if (task.has ("parent"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_PARENT);
      view.set (row, 1, task.get ("parent"));
    }

    // mask
    if (task.has ("mask"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_MASK);
      view.set (row, 1, task.get ("mask"));
    }

    // imask
    if (task.has ("imask"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_MASK_IDX);
      view.set (row, 1, task.get ("imask"));
    }

    // entry
    row = view.addRow ();
    view.set (row, 0, STRING_COLUMN_LABEL_ENTERED);
    ISO8601d dt (task.get_date ("entry"));
    std::string entry = dt.toString (dateformat);

    std::string age;
    std::string created = task.get ("entry");
    if (created.length ())
    {
      ISO8601d dt (strtol (created.c_str (), NULL, 10));
      age = ISO8601p (now - dt).formatVague ();
    }

    view.set (row, 1, entry + " (" + age + ")");

    // wait
    if (task.has ("wait"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_WAITING);
      view.set (row, 1, ISO8601d (task.get_date ("wait")).toString (dateformat));
    }

    // scheduled
    if (task.has ("scheduled"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_SCHED);
      view.set (row, 1, ISO8601d (task.get_date ("scheduled")).toString (dateformat));
    }

    // start
    if (task.has ("start"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_START);
      view.set (row, 1, ISO8601d (task.get_date ("start")).toString (dateformat));
    }

    // due (colored)
    if (task.has ("due"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_DUE);
      view.set (row, 1, ISO8601d (task.get_date ("due")).toString (dateformat));
    }

    // end
    if (task.has ("end"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_END);
      view.set (row, 1, ISO8601d (task.get_date ("end")).toString (dateformat));
    }

    // until
    if (task.has ("until"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_CMD_INFO_UNTIL);
      view.set (row, 1, ISO8601d (task.get_date ("until")).toString (dateformat));
    }

    // modified
    if (task.has ("modified"))
    {
      row = view.addRow ();
      view.set (row, 0, STRING_CMD_INFO_MODIFIED);

      ISO8601d mod (task.get_date ("modified"));
      std::string age = ISO8601p (now - mod).formatVague ();
      view.set (row, 1, mod.toString (dateformat) + " (" + age + ")");
    }

    // tags ...
    std::vector <std::string> tags;
    task.getTags (tags);
    if (tags.size ())
    {
      std::string allTags;
      join (allTags, " ", tags);

      row = view.addRow ();
      view.set (row, 0, STRING_COLUMN_LABEL_TAGS);
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
      if (task.hasTag ("CHILD"))     virtualTags += "CHILD ";
      if (task.hasTag ("COMPLETED")) virtualTags += "COMPLETED ";
      if (task.hasTag ("DELETED"))   virtualTags += "DELETED ";
      if (task.hasTag ("DUE"))       virtualTags += "DUE ";
      if (task.hasTag ("DUETODAY"))  virtualTags += "DUETODAY ";
      if (task.hasTag ("MONTH"))     virtualTags += "MONTH ";
      if (task.hasTag ("ORPHAN"))    virtualTags += "ORPHAN ";
      if (task.hasTag ("OVERDUE"))   virtualTags += "OVERDUE ";
      if (task.hasTag ("PARENT"))    virtualTags += "PARENT ";
      if (task.hasTag ("PENDING"))   virtualTags += "PENDING ";
      if (task.hasTag ("READY"))     virtualTags += "READY ";
      if (task.hasTag ("SCHEDULED")) virtualTags += "SCHEDULED ";
      if (task.hasTag ("TAGGED"))    virtualTags += "TAGGED ";
      if (task.hasTag ("TODAY"))     virtualTags += "TODAY ";
      if (task.hasTag ("TOMORROW"))  virtualTags += "TOMORROW ";
      if (task.hasTag ("UDA"))       virtualTags += "UDA ";
      if (task.hasTag ("UNBLOCKED")) virtualTags += "UNBLOCKED ";
      if (task.hasTag ("UNTIL"))     virtualTags += "UNTIL ";
      if (task.hasTag ("WAITING"))   virtualTags += "WAITING ";
      if (task.hasTag ("WEEK"))      virtualTags += "WEEK ";
      if (task.hasTag ("YEAR"))      virtualTags += "YEAR ";
      if (task.hasTag ("YESTERDAY")) virtualTags += "YESTERDAY ";
      if (task.hasTag ("LATEST"))    virtualTags += "LATEST ";
      if (task.hasTag ("PROJECT"))   virtualTags += "PROJECT ";
      if (task.hasTag ("PRIORITY"))  virtualTags += "PRIORITY ";
      // If you update the above list, update src/commands/CmdInfo.cpp and src/commands/CmdTags.cpp as well.

      row = view.addRow ();
      view.set (row, 0, STRING_CMD_INFO_VIRTUAL_TAGS);
      view.set (row, 1, virtualTags);
    }

    // uuid
    row = view.addRow ();
    view.set (row, 0, STRING_COLUMN_LABEL_UUID);
    std::string uuid = task.get ("uuid");
    view.set (row, 1, uuid);

    // Task::urgency
    row = view.addRow ();
    view.set (row, 0, STRING_COLUMN_LABEL_URGENCY);
    view.set (row, 1, format (task.urgency (), 4, 4));

    // Show any UDAs
    std::vector <std::string> all = task.all ();
    std::string type;
    for (auto& att: all)
    {
      if (context.columns.find (att) != context.columns.end ())
      {
        Column* col = context.columns[att];
        if (col->is_uda ())
        {
          std::string value = task.get (att);
          if (value != "")
          {
            row = view.addRow ();
            view.set (row, 0, col->label ());

            if (type == "date")
              value = ISO8601d (value).toString (dateformat);
            else if (type == "duration")
            {
              ISO8601p iso;
              std::string::size_type cursor = 0;
              if (iso.parse (value, cursor))
                value = (std::string) Variant ((time_t) iso, Variant::type_duration);
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
          context.columns.find (att) == context.columns.end ())
      {
         row = view.addRow ();
         view.set (row, 0, "[" + att);
         view.set (row, 1, task.get (att) + "]");
      }
    }

    // Create a second table, containing urgency details, if necessary.
    ViewText urgencyDetails;
    if (task.urgency () != 0.0)
    {
      if (context.color ())
      {
        Color alternate (context.config.get ("color.alternate"));
        urgencyDetails.colorOdd (alternate);
        urgencyDetails.intraColorOdd (alternate);

        Color label (context.config.get ("color.label"));
        urgencyDetails.colorHeader (label);
      }

      urgencyDetails.width (context.getWidth ());
      urgencyDetails.add (Column::factory ("string", "")); // Attribute
      urgencyDetails.add (Column::factory ("string", "")); // Value
      urgencyDetails.add (Column::factory ("string", "")); // *
      urgencyDetails.add (Column::factory ("string", "")); // Coefficient
      urgencyDetails.add (Column::factory ("string", "")); // =
      urgencyDetails.add (Column::factory ("string", "")); // Result

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
            std::string project = var.first.substr (21, end - 21);
            if (task.get ("project").find (project) == 0)
              urgencyTerm (urgencyDetails, "PROJECT " + project, 1.0, var.second);
          }

          // urgency.user.tag.<tag>.coefficient
          if (var.first.substr (13, 4) == "tag." &&
              (end = var.first.find (".coefficient")) != std::string::npos)
          {
            std::string name = var.first.substr (17, end - 17);
            if (task.hasTag (name))
              urgencyTerm (urgencyDetails, "TAG " + name, 1.0, var.second);
          }

          // urgency.user.keyword.<keyword>.coefficient
          if (var.first.substr (13, 8) == "keyword." &&
              (end = var.first.find (".coefficient")) != std::string::npos)
          {
            std::string keyword = var.first.substr (21, end - 21);
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
            const std::string uda = var.first.substr (12, end - 12);
            auto dot = uda.find (".");
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
    ViewText journal;

    // If an alternating row color is specified, notify the table.
    if (context.color ())
    {
      Color alternate (context.config.get ("color.alternate"));
      journal.colorOdd (alternate);
      journal.intraColorOdd (alternate);

      Color label (context.config.get ("color.label"));
      journal.colorHeader (label);
    }

    journal.width (context.getWidth ());
    journal.add (Column::factory ("string", STRING_COLUMN_LABEL_DATE));
    journal.add (Column::factory ("string", STRING_CMD_INFO_MODIFICATION));

    if (context.config.getBoolean ("journal.info") &&
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

            ISO8601d timestamp (strtol (undo[when].substr (5).c_str (), NULL, 10));
            journal.set (row, 0, timestamp.toString (dateformat));

            Task before (undo[previous].substr (4));
            Task after (undo[current].substr (4));
            journal.set (row, 1, taskInfoDifferences (before, after, dateformat, last_timestamp, timestamp.toEpoch()));
          }
        }
      }
    }

    out << optionalBlankLine ()
        << view.render ()
        << "\n";

    if (urgencyDetails.rows () > 0)
      out << urgencyDetails.render ()
          << "\n";

    if (journal.rows () > 0)
      out << journal.render ()
          << "\n";
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void CmdInfo::urgencyTerm (
  ViewText& view,
  const std::string& label,
  float measure,
  float coefficient) const
{
  float value = measure * coefficient;
  if (value != 0.0)
  {
    int row = view.addRow ();
    view.set (row, 0, "    " + label);
    view.set (row, 1, rightJustify (format (measure, 5, 3), 6));
    view.set (row, 2, "*");
    view.set (row, 3, rightJustify (format (coefficient, 4, 2), 4));
    view.set (row, 4, "=");
    view.set (row, 5, rightJustify (format (value, 5, 3), 6));
  }
}

////////////////////////////////////////////////////////////////////////////////
