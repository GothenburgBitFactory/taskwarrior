////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <Nibbler.h>
#include <File.h>
#include <Date.h>
#include <text.h>
#include <util.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
enum fileType
{
  not_a_clue,
  task_1_4_3,
  task_1_5_0,
  task_1_6_0,
  task_cmd_line,
  todo_sh_2_0,
  csv,
  yaml,
  text
};

static fileType determineFileType (const std::vector <std::string>& lines)
{
  // '7f7a4191-c2f2-487f-8855-7a1eb378c267',' ...
  // ....:....|....:....|....:....|....:....|
  // 1        10        20        30        40
  if (lines.size () > 1    &&
      lines[1][0]  == '\'' &&
      lines[1][9]  == '-'  &&
      lines[1][14] == '-'  &&
      lines[1][19] == '-'  &&
      lines[1][24] == '-'  &&
      lines[1][37] == '\'' &&
      lines[1][38] == ','  &&
      lines[1][39] == '\'')
  {
    if (lines[0] == "'uuid','status','tags','entry','start','due','recur',"
                    "'end','project','priority','fg','bg','description'")
      return task_1_6_0;

    if (lines[0] == "'id','uuid','status','tags','entry','start','due','recur',"
                    "'end','project','priority','fg','bg','description'")
      return task_1_5_0;

    if (lines[0] == "'id','status','tags','entry','start','due','end','project',"
                    "'priority','fg','bg','description'")
      return task_1_4_3;
  }

  // A task command line might include a priority or project.
  for (unsigned int i = 0; i < lines.size (); ++i)
  {
    std::vector <std::string> words;
    split (words, lines[i], ' ');

    for (unsigned int w = 0; w < words.size (); ++w)
      if (words[w].substr (0, 9) == "priority:" ||
          words[w].substr (0, 8) == "priorit:"  ||
          words[w].substr (0, 7) == "priori:"   ||
          words[w].substr (0, 6) == "prior:"    ||
          words[w].substr (0, 5) == "prio:"     ||
          words[w].substr (0, 4) == "pri:"      ||
          words[w].substr (0, 8) == "project:"  ||
          words[w].substr (0, 7) == "projec:"   ||
          words[w].substr (0, 6) == "proje:"    ||
          words[w].substr (0, 5) == "proj:"     ||
          words[w].substr (0, 4) == "pro:")
        return task_cmd_line;
  }

  // x 2009-03-25 Walk the dog +project @context
  // This is a test +project @context
  for (unsigned int i = 0; i < lines.size (); ++i)
  {
    // All done tasks begin with "x YYYY-MM-DD".
    if (lines[i].length () > 12)
    {
      if (           lines[i][0] == 'x' &&
                     lines[i][1] == ' ' &&
          isdigit (lines[i][2]) &&
          isdigit (lines[i][3]) &&
          isdigit (lines[i][4]) &&
          isdigit (lines[i][5]) &&
                     lines[i][6] == '-' &&
          isdigit (lines[i][7]) &&
          isdigit (lines[i][8]) &&
                     lines[i][9] == '-' &&
          isdigit (lines[i][10]) &&
          isdigit (lines[i][11]))
        return todo_sh_2_0;
    }

    std::vector <std::string> words;
    split (words, lines[i], ' ');
    for (unsigned int w = 0; w < words.size (); ++w)
    {
      // +project
      if (words[w].length () > 1 &&
          words[w][0] == '+'     &&
          !isspace (words[w][1]))
        return todo_sh_2_0;

      // @context
      if (words[w].length () > 1 &&
          words[w][0] == '@'     &&
          !isspace (words[w][1]))
        return todo_sh_2_0;
    }
  }

  // CSV - commas on every non-comment, non-trivial line.
  bool commas_on_every_line = true;
  for (unsigned int i = 0; i < lines.size (); ++i)
  {
    if (lines[i].length () > 10 &&
        lines[i].find (",") == std::string::npos)
    {
      commas_on_every_line = false;
      break;
    }
  }
  if (commas_on_every_line)
    return csv;

  if (lines.size () > 2 &&
      lines[0] == "% YAML 1.1\n" &&
      lines[1] == "---\n")
  {
    return yaml;
  }

  // Looks like 'text' is the default case, if there is any data at all.
  if (lines.size () > 1)
    return text;

  return not_a_clue;
}

////////////////////////////////////////////////////////////////////////////////
static void decorateTask (Task& task)
{
  char entryTime[16];
  sprintf (entryTime, "%u", (unsigned int) time (NULL));
  task.set ("entry", entryTime);

  task.setStatus (Task::pending);

  // Override with default.project, if not specified.
  std::string defaultProject = context.config.get ("default.project");
  if (!task.has ("project") && defaultProject  != "")
    task.set ("project", defaultProject);

  // Override with default.priority, if not specified.
  std::string defaultPriority = context.config.get ("default.priority");
  if (!task.has ("priority") &&
      defaultPriority != "" &&
      Att::validNameValue ("priority", "", defaultPriority))
    task.set ("priority", defaultPriority);
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTask_1_4_3 (const std::vector <std::string>& lines)
{
  std::vector <std::string> failed;

  context.tdb.lock (context.config.getBoolean ("locking"));

  std::vector <std::string>::const_iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    try
    {
      // Skip the first line, if it is a columns header line.
      if (it->substr (0, 5) == "'id',")
        continue;

      std::vector <std::string> fields;
      split (fields, *it, ',');

      // If there is an unexpected number of fields, something is wrong.  Perhaps
      // an embedded comma, in which case there are (at least) two fields that
      // need to be concatenated.
      if (fields.size () > 12)
      {
        int safety = 10;  // Shouldn't be more than 10 commas in a description/project.

        do
        {
          std::vector <std::string> modified;
          for (unsigned int f = 0; f < fields.size (); ++f)
          {
            if (fields[f][0]                       != '\'' &&
                fields[f][fields[f].length () - 1] == '\'')
            {
              modified[modified.size () - 1] += "," + fields[f];
            }

            else
              modified.push_back (fields[f]);
          }
          fields = modified;

          if (safety-- <= 0)
            throw "unrecoverable";
        }
        while (fields.size () > 12);
      }

      if (fields.size () < 12)
        throw "unrecoverable";

      // Build up this task ready for insertion.
      Task task;

      // Handle the 12 fields.
      for (unsigned int f = 0; f < fields.size (); ++f)
      {
        switch (f)
        {
        case 0: // 'uuid'
          task.set ("uuid", fields[f].substr (1, 36));
          break;

        case 1: // 'status'
               if (fields[f] == "'pending'")   task.setStatus (Task::pending);
          else if (fields[f] == "'recurring'") task.setStatus (Task::recurring);
          else if (fields[f] == "'deleted'")   task.setStatus (Task::deleted);
          else if (fields[f] == "'completed'") task.setStatus (Task::completed);
          break;

        case 2: // 'tags'
          if (fields[f].length () > 2)
          {
            std::string tokens = fields[f].substr (1, fields[f].length () - 2);
            std::vector <std::string> tags;
            split (tags, tokens, ' ');
            for (unsigned int i = 0; i < tags.size (); ++i)
              task.addTag (tags[i]);
          }
          break;

        case 3: // entry
          task.set ("entry", fields[f]);
          break;

        case 4: // start
          if (fields[f].length ())
            task.set ("start", fields[f]);
          break;

        case 5: // due
          if (fields[f].length ())
            task.set ("due", fields[f]);
          break;

        case 6: // end
          if (fields[f].length ())
            task.set ("end", fields[f]);
          break;

        case 7: // 'project'
          if (fields[f].length () > 2)
            task.set ("project", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 8: // 'priority'
          if (fields[f].length () > 2)
            task.set ("priority", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 9: // 'fg'
          if (fields[f].length () > 2)
            task.set ("fg", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 10: // 'bg'
          if (fields[f].length () > 2)
            task.set ("bg", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 11: // 'description'
          if (fields[f].length () > 2)
            task.set ("description", fields[f].substr (1, fields[f].length () - 2));
          break;
        }
      }

      context.tdb.add (task);
    }

    catch (...)
    {
      failed.push_back (*it);
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  std::stringstream out;
  out << "Imported "
      << (lines.size () - failed.size () - 1)
      << " tasks successfully, with "
      << failed.size ()
      << " errors."
      << std::endl;

  if (failed.size ())
  {
    std::string bad;
    join (bad, "\n", failed);
    return out.str () + "\nCould not import:\n\n" + bad;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTask_1_5_0 (const std::vector <std::string>& lines)
{
  std::vector <std::string> failed;

  context.tdb.lock (context.config.getBoolean ("locking"));

  std::vector <std::string>::const_iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    try
    {
      // Skip the first line, if it is a columns header line.
      if (it->substr (0, 5) == "'id',")
        continue;

      std::vector <std::string> fields;
      split (fields, *it, ',');

      // If there is an unexpected number of fields, something is wrong.  Perhaps
      // an embedded comma, in which case there are (at least) two fields that
      // need to be concatenated.
      if (fields.size () > 13)
      {
        int safety = 10;  // Shouldn't be more than 10 commas in a description/project.

        do
        {
          std::vector <std::string> modified;
          for (unsigned int f = 0; f < fields.size (); ++f)
          {
            if (fields[f][0]                       != '\'' &&
                fields[f][fields[f].length () - 1] == '\'')
            {
              modified[modified.size () - 1] += "," + fields[f];
            }

            else
              modified.push_back (fields[f]);
          }
          fields = modified;

          if (safety-- <= 0)
            throw "unrecoverable";
        }
        while (fields.size () > 13);
      }

      if (fields.size () < 13)
        throw "unrecoverable";

      // Build up this task ready for insertion.
      Task task;

      // Handle the 13 fields.
      for (unsigned int f = 0; f < fields.size (); ++f)
      {
        switch (f)
        {
        case 0: // 'uuid'
          task.set ("uuid", fields[f].substr (1, 36));
          break;

        case 1: // 'status'
               if (fields[f] == "'pending'")   task.setStatus (Task::pending);
          else if (fields[f] == "'recurring'") task.setStatus (Task::recurring);
          else if (fields[f] == "'deleted'")   task.setStatus (Task::deleted);
          else if (fields[f] == "'completed'") task.setStatus (Task::completed);
          break;

        case 2: // 'tags'
          if (fields[f].length () > 2)
          {
            std::string tokens = fields[f].substr (1, fields[f].length () - 2);
            std::vector <std::string> tags;
            split (tags, tokens, ' ');
            for (unsigned int i = 0; i < tags.size (); ++i)
              task.addTag (tags[i]);
          }
          break;

        case 3: // entry
          task.set ("entry", fields[f]);
          break;

        case 4: // start
          if (fields[f].length ())
            task.set ("start", fields[f]);
          break;

        case 5: // due
          if (fields[f].length ())
            task.set ("due", fields[f]);
          break;

        case 6: // recur
          if (fields[f].length ())
            task.set ("recur", fields[f]);
          break;

        case 7: // end
          if (fields[f].length ())
            task.set ("end", fields[f]);
          break;

        case 8: // 'project'
          if (fields[f].length () > 2)
            task.set ("project", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 9: // 'priority'
          if (fields[f].length () > 2)
            task.set ("priority", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 10: // 'fg'
          if (fields[f].length () > 2)
            task.set ("fg", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 11: // 'bg'
          if (fields[f].length () > 2)
            task.set ("bg", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 12: // 'description'
          if (fields[f].length () > 2)
            task.set ("description", fields[f].substr (1, fields[f].length () - 2));
          break;
        }
      }

      context.tdb.add (task);
    }

    catch (...)
    {
      failed.push_back (*it);
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  std::stringstream out;
  out << "Imported "
      << (lines.size () - failed.size () - 1)
      << " tasks successfully, with "
      << failed.size ()
      << " errors."
      << std::endl;

  if (failed.size ())
  {
    std::string bad;
    join (bad, "\n", failed);
    return out.str () + "\nCould not import:\n\n" + bad;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTask_1_6_0 (const std::vector <std::string>& lines)
{
  std::vector <std::string> failed;

  context.tdb.lock (context.config.getBoolean ("locking"));

  std::vector <std::string>::const_iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    try
    {
      // Skip the first line, if it is a columns header line.
      if (it->substr (0, 7) == "'uuid',")
        continue;

      std::vector <std::string> fields;
      split (fields, *it, ',');

      // If there is an unexpected number of fields, something is wrong.  Perhaps
      // an embedded comma, in which case there are (at least) two fields that
      // need to be concatenated.
      if (fields.size () > 13)
      {
        int safety = 10;  // Shouldn't be more than 10 commas in a description/project.

        do
        {
          std::vector <std::string> modified;
          for (unsigned int f = 0; f < fields.size (); ++f)
          {
            if (fields[f][0]                       != '\'' &&
                fields[f][fields[f].length () - 1] == '\'')
            {
              modified[modified.size () - 1] += "," + fields[f];
            }

            else
              modified.push_back (fields[f]);
          }
          fields = modified;

          if (safety-- <= 0)
            throw "unrecoverable";
        }
        while (fields.size () > 13);
      }

      if (fields.size () < 13)
        throw "unrecoverable";

      // Build up this task ready for insertion.
      Task task;

      // Handle the 13 fields.
      for (unsigned int f = 0; f < fields.size (); ++f)
      {
        switch (f)
        {
        case 0: // 'uuid'
          task.set ("uuid", fields[f].substr (1, 36));
          break;

        case 1: // 'status'
               if (fields[f] == "'pending'")   task.setStatus (Task::pending);
          else if (fields[f] == "'recurring'") task.setStatus (Task::recurring);
          else if (fields[f] == "'deleted'")   task.setStatus (Task::deleted);
          else if (fields[f] == "'completed'") task.setStatus (Task::completed);
          else if (fields[f] == "'waiting'")   task.setStatus (Task::waiting);
          break;

        case 2: // 'tags'
          if (fields[f].length () > 2)
          {
            std::string tokens = fields[f].substr (1, fields[f].length () - 2);
            std::vector <std::string> tags;
            split (tags, tokens, ' ');
            for (unsigned int i = 0; i < tags.size (); ++i)
              task.addTag (tags[i]);
          }
          break;

        case 3: // entry
          task.set ("entry", fields[f]);
          break;

        case 4: // start
          if (fields[f].length ())
            task.set ("start", fields[f]);
          break;

        case 5: // due
          if (fields[f].length ())
            task.set ("due", fields[f]);
          break;

        case 6: // recur
          if (fields[f].length ())
            task.set ("recur", fields[f]);
          break;

        case 7: // end
          if (fields[f].length ())
            task.set ("end", fields[f]);
          break;

        case 8: // 'project'
          if (fields[f].length () > 2)
            task.set ("project", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 9: // 'priority'
          if (fields[f].length () > 2)
            task.set ("priority", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 10: // 'fg'
          if (fields[f].length () > 2)
            task.set ("fg", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 11: // 'bg'
          if (fields[f].length () > 2)
            task.set ("bg", fields[f].substr (1, fields[f].length () - 2));
          break;

        case 12: // 'description'
          if (fields[f].length () > 2)
            task.set ("description", fields[f].substr (1, fields[f].length () - 2));
          break;
        }
      }

      context.tdb.add (task);
    }

    catch (...)
    {
      failed.push_back (*it);
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  std::stringstream out;
  out << "Imported "
      << (lines.size () - failed.size () - 1)
      << " tasks successfully, with "
      << failed.size ()
      << " errors."
      << std::endl;

  if (failed.size ())
  {
    std::string bad;
    join (bad, "\n", failed);
    return out.str () + "\nCould not import:\n\n" + bad;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTaskCmdLine (const std::vector <std::string>& lines)
{
  std::vector <std::string> failed;
  std::string unused;

  std::vector <std::string>::const_iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    std::string line = *it;

    try
    {
      context.args.clear ();
      split (context.args, std::string ("add ") + line, ' ');

      context.task.clear ();
      context.cmd.command = "";
      context.parse ();
      (void)handleAdd (unused);
      context.clearMessages ();
    }

    catch (...)
    {
      context.clearMessages ();
      failed.push_back (line);
    }
  }

  std::stringstream out;
  out << "Imported "
      << (lines.size () - failed.size ())
      << " tasks successfully, with "
      << failed.size ()
      << " errors."
      << std::endl;

  if (failed.size ())
  {
    std::string bad;
    join (bad, "\n", failed);
    return out.str () + "\nCould not import:\n\n" + bad;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
static std::string importTodoSh_2_0 (const std::vector <std::string>& lines)
{
  std::vector <std::string> failed;

  context.tdb.lock (context.config.getBoolean ("locking"));

  std::vector <std::string>::const_iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    try
    {
      context.args.clear ();
      context.args.push_back ("add");

      bool isPending = true;
      Date endDate;

      std::vector <std::string> words;
      split (words, *it, ' ');

      for (unsigned int w = 0; w < words.size (); ++w)
      {
        if (words[w].length () > 1 &&
            words[w][0] == '+')
        {
          context.args.push_back (std::string ("project:") + words[w].substr (1));
        }

        // Convert "+aaa" to "project:aaa".
        // Convert "@aaa" to "+aaa".
        else if (words[w].length () > 1 &&
                 words[w][0] == '@')
        {
          context.args.push_back (std::string ("+") + words[w].substr (1));
        }

        // Convert "(A)" to "priority:H".
        // Convert "(B)" to "priority:M".
        // Convert "(?)" to "priority:L".
        else if (words[w].length () == 3 &&
                 words[w][0] == '('      &&
                 words[w][2] == ')')
        {
               if (words[w][1] == 'A') context.args.push_back ("priority:H");
          else if (words[w][1] == 'B') context.args.push_back ("priority:M");
          else                         context.args.push_back ("priority:L");
        }

        // Set status, if completed.
        else if (w == 0 &&
                 words[w] == "x")
        {
          isPending = false;
        }

        // Set status, and add an end date, if completed.
        else if (! isPending &&
                 w == 1 &&
                 words[w].length () == 10 &&
                 words[w][4] == '-'       &&
                 words[w][7] == '-')
        {
          endDate = Date (words[w], "Y-M-D");
        }

        // Just an ordinary word.
        else
        {
          context.args.push_back (words[w]);
        }
      }

      context.task.clear ();
      context.cmd.command = "";
      context.parse ();
      decorateTask (context.task);

      context.task.set ("uuid", uuid ());

      if (isPending)
      {
        context.task.setStatus (Task::pending);
      }
      else
      {
        context.task.setStatus (Task::completed);

        char end[16];
        sprintf (end, "%u", (unsigned int) endDate.toEpoch ());
        context.task.set ("end", end);
      }

      context.tdb.add (context.task);
      context.clearMessages ();
    }

    catch (...)
    {
      context.clearMessages ();
      failed.push_back (*it);
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  std::stringstream out;
  out << "Imported "
      << (lines.size () - failed.size ())
      << " tasks successfully, with "
      << failed.size ()
      << " errors."
      << std::endl;

  if (failed.size ())
  {
    std::string bad;
    join (bad, "\n", failed);
    return out.str () + "\nCould not import:\n\n" + bad;
  }
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
static std::string importText (const std::vector <std::string>& lines)
{
  std::vector <std::string> failed;
  int count = 0;

  context.tdb.lock (context.config.getBoolean ("locking"));

  std::vector <std::string>::const_iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    std::string line = *it;

    // Strip comments
    std::string::size_type pound = line.find ("#");
    if (pound != std::string::npos)
      line = line.substr (0, pound);

    // Skip blank lines
    if (line.length () > 0)
    {
      try
      {
        ++count;
        context.args.clear ();
        split (context.args, std::string ("add ") + line, ' ');

        context.task.clear ();
        context.cmd.command = "";
        context.parse ();
        decorateTask (context.task);

        context.tdb.add (context.task);
        context.clearMessages ();
      }

      catch (...)
      {
        context.clearMessages ();
        failed.push_back (line);
      }
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  std::stringstream out;
  out << "Imported "
      << count
      << " tasks successfully, with "
      << failed.size ()
      << " errors."
      << std::endl;

  if (failed.size ())
  {
    std::string bad;
    join (bad, "\n", failed);
    return out.str () + "\nCould not import:\n\n" + bad;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
static std::string importCSV (const std::vector <std::string>& lines)
{
  std::vector <std::string> failed;

  context.tdb.lock (context.config.getBoolean ("locking"));

  // Set up mappings.  Assume no fields match.
  std::map <std::string, int> mapping;
  mapping ["id"]          = -1;
  mapping ["uuid"]        = -1;
  mapping ["status"]      = -1;
  mapping ["tags"]        = -1;
  mapping ["entry"]       = -1;
  mapping ["start"]       = -1;
  mapping ["due"]         = -1;
  mapping ["recur"]       = -1;
  mapping ["end"]         = -1;
  mapping ["project"]     = -1;
  mapping ["priority"]    = -1;
  mapping ["fg"]          = -1;
  mapping ["bg"]          = -1;
  mapping ["description"] = -1;

  std::vector <std::string> headings;
  split (headings, lines[0], ',');

  for (unsigned int h = 0; h < headings.size (); ++h)
  {
    std::string name = lowerCase (trim (unquoteText (trim (headings[h]))));

    // If there is a mapping for the field, use the value.
    if (name == context.config.get ("import.synonym.id") ||
        name == "id" ||
        name == "#" ||
        name == "sequence" ||
        name.find ("num") != std::string::npos)
    {
      mapping["id"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.uuid") ||
             name == "uuid" ||
             name == "guid" ||
             name.find ("unique") != std::string::npos)
    {
      mapping["uuid"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.status") ||
             name == "status" ||
             name == "condition" ||
             name == "state")
    {
      mapping["status"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.tags") ||
             name == "tags" ||
             name.find ("categor") != std::string::npos ||
             name.find ("tag") != std::string::npos)
    {
      mapping["tags"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.entry") ||
             name == "entry" ||
             name.find ("added") != std::string::npos ||
             name.find ("created") != std::string::npos ||
             name.find ("entered") != std::string::npos)
    {
      mapping["entry"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.start") ||
             name == "start" ||
             name.find ("began") != std::string::npos ||
             name.find ("begun") != std::string::npos ||
             name.find ("started") != std::string::npos)
    {
      mapping["start"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.due") ||
             name == "due" ||
             name.find ("expected") != std::string::npos)
    {
      mapping["due"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.recur") ||
             name == "recur" ||
             name == "frequency")
    {
      mapping["recur"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.end") ||
             name == "end" ||
             name == "done" ||
             name.find ("complete") != std::string::npos)
    {
      mapping["end"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.project") ||
             name == "project" ||
             name.find ("proj") != std::string::npos)
    {
      mapping["project"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.priority") ||
             name == "priority" ||
             name == "pri" ||
             name.find ("importan") != std::string::npos)
    {
      mapping["priority"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.fg") ||
             name.find ("fg")         != std::string::npos ||
             name.find ("foreground") != std::string::npos ||
             name.find ("color")      != std::string::npos)
    {
      mapping["fg"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.bg") ||
             name == "bg" ||
             name.find ("background") != std::string::npos)
    {
      mapping["bg"] = (int)h;
    }

    else if (name == context.config.get ("import.synonym.description") ||
             name.find ("desc")   != std::string::npos ||
             name.find ("detail") != std::string::npos ||
             name.find ("task")   != std::string::npos ||
             name.find ("what")   != std::string::npos)
    {
      mapping["description"] = (int)h;
    }
  }

  // TODO Dump mappings and ask for confirmation?

  std::vector <std::string>::const_iterator it = lines.begin ();
  for (++it; it != lines.end (); ++it)
  {
    try
    {
      std::vector <std::string> fields;
      split (fields, *it, ',');

      Task task;

      int f;
      if ((f = mapping["uuid"]) != -1)
        task.set ("uuid", lowerCase (unquoteText (trim (fields[f]))));

      task.setStatus (Task::pending);
      if ((f = mapping["status"]) != -1)
      {
        std::string value = lowerCase (unquoteText (trim (fields[f])));

             if (value == "recurring") task.setStatus (Task::recurring);
        else if (value == "deleted")   task.setStatus (Task::deleted);
        else if (value == "completed") task.setStatus (Task::completed);
        else if (value == "waiting")   task.setStatus (Task::waiting);
      }

      if ((f = mapping["tags"]) != -1)
      {
        std::string value = unquoteText (trim (fields[f]));
        std::vector <std::string> tags;
        split (tags, value, ' ');
        for (unsigned int i = 0; i < tags.size (); ++i)
          task.addTag (tags[i]);
      }

      if ((f = mapping["entry"]) != -1)
        task.set ("entry", lowerCase (unquoteText (trim (fields[f]))));

      if ((f = mapping["start"]) != -1)
        task.set ("start", lowerCase (unquoteText (trim (fields[f]))));

      if ((f = mapping["due"]) != -1)
        task.set ("due", lowerCase (unquoteText (trim (fields[f]))));

      if ((f = mapping["recur"]) != -1)
        task.set ("recur", lowerCase (unquoteText (trim (fields[f]))));

      if ((f = mapping["end"]) != -1)
        task.set ("end", lowerCase (unquoteText (trim (fields[f]))));

      if ((f = mapping["project"]) != -1)
        task.set ("project", unquoteText (trim (fields[f])));

      if ((f = mapping["priority"]) != -1)
      {
        std::string value = upperCase (unquoteText (trim (fields[f])));
        if (value == "H" || value == "M" || value == "L")
          task.set ("priority", value);
      }

      if ((f = mapping["fg"]) != -1)
        task.set ("fg", lowerCase (unquoteText (trim (fields[f]))));

      if ((f = mapping["bg"]) != -1)
        task.set ("bg", lowerCase (unquoteText (trim (fields[f]))));

      if ((f = mapping["description"]) != -1)
        task.set ("description", unquoteText (trim (fields[f])));

      context.tdb.add (task);
    }

    catch (...)
    {
      failed.push_back (*it);
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  std::stringstream out;
  out << "Imported "
      << (lines.size () - failed.size () - 1)
      << " tasks successfully, with "
      << failed.size ()
      << " errors."
      << std::endl;

  if (failed.size ())
  {
    std::string bad;
    join (bad, "\n", failed);
    return out.str () + "\nCould not import:\n\n" + bad;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
int handleImport (std::string &outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-import-command"))
  {
    std::stringstream out;

    // Use the description as a file name.
    std::string file = trim (context.task.get ("description"));
    if (file.length () > 0)
    {
      // Load the file.
      std::vector <std::string> all;
      File::read (file, all);

      std::vector <std::string> lines;
      std::vector <std::string>::iterator it;
      for (it = all.begin (); it != all.end (); ++it)
      {
        std::string line = *it;

        // Strip comments
        std::string::size_type pound = line.find ("#");
        if (pound != std::string::npos)
          line = line.substr (0, pound);

        trim (line);

        // Skip blank lines
        if (line.length () > 0)
          lines.push_back (line);
      }

      // Take a guess at the file type.
      fileType type = determineFileType (lines);
      std::string identifier;
      switch (type)
      {
      case task_1_4_3:    identifier = "This looks like an older task export file.";              break;
      case task_1_5_0:    identifier = "This looks like a recent task export file.";              break;
      case task_1_6_0:    identifier = "This looks like a current task export file.";             break;
      case task_cmd_line: identifier = "This looks like task command line arguments.";            break;
      case todo_sh_2_0:   identifier = "This looks like a todo.sh 2.x file.";                     break;
      case csv:           identifier = "This looks like a CSV file, but not a task export file."; break;
      case yaml:          identifier = "This looks like a YAML file.";                            break;
      case text:          identifier = "This looks like a text file with one task per line.";     break;
      case not_a_clue:
        throw std::string ("Task cannot determine which type of file this is, "
                           "and cannot proceed.");
      }

      // For tty users, confirm the import, as it is destructive.
      if (isatty (fileno (stdout)))
        if (! confirm (identifier + "  Okay to proceed?"))
          throw std::string ("Task will not import any data.");

      // Determine which type it might be, then attempt an import.
      switch (type)
      {
      case task_1_4_3:    out << importTask_1_4_3  (lines); break;
      case task_1_5_0:    out << importTask_1_5_0  (lines); break;
      case task_1_6_0:    out << importTask_1_6_0  (lines); break;
      case task_cmd_line: out << importTaskCmdLine (lines); break;
      case todo_sh_2_0:   out << importTodoSh_2_0  (lines); break;
      case csv:           out << importCSV         (lines); break;
      case yaml:          throw std::string ("import.yaml not implemented.");
      case text:          out << importText        (lines); break;
      case not_a_clue:    /* to stop the compiler from complaining. */ break;
      }
    }
    else
      throw std::string ("You must specify a file to import.");

    outs = out.str ();
    context.hooks.trigger ("post-import-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
