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
#include <CmdSummary.h>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include <Context.h>
#include <Filter.h>
#include <ViewText.h>
#include <ISO8601.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdSummary::CmdSummary ()
{
  _keyword               = "summary";
  _usage                 = "task <filter> summary";
  _description           = STRING_CMD_SUMMARY_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
// Project  Remaining  Avg Age  Complete  0%                  100%
// A               12      13d       55%  XXXXXXXXXXXXX-----------
// B              109   3d 12h       10%  XXX---------------------
int CmdSummary::execute (std::string& output)
{
  int rc = 0;
  bool showAllProjects = context.config.getBoolean ("summary.all.projects");

  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Generate unique list of project names from all pending tasks.
  std::map <std::string, bool> allProjects;
  for (auto& task : filtered)
    if (showAllProjects || task.getStatus () == Task::pending)
      allProjects[task.get ("project")] = false;

  // Initialize counts, sum.
  std::map <std::string, int> countPending;
  std::map <std::string, int> countCompleted;
  std::map <std::string, double> sumEntry;
  std::map <std::string, int> counter;
  time_t now = time (NULL);

  // Initialize counters.
  for (auto& project : allProjects)
  {
    countPending   [project.first] = 0;
    countCompleted [project.first] = 0;
    sumEntry       [project.first] = 0.0;
    counter        [project.first] = 0;
  }

  // Count the various tasks.
  for (auto& task : filtered)
  {
    std::string project = task.get ("project");
    std::vector <std::string> projects = extractParents (project);
    projects.push_back (project);

    for (auto& parent : projects)
      ++counter[parent];

    if (task.getStatus () == Task::pending ||
        task.getStatus () == Task::waiting)
    {
      for (auto& parent : projects)
      {
        ++countPending[parent];

        time_t entry = strtol (task.get ("entry").c_str (), NULL, 10);
        if (entry)
          sumEntry[parent] = sumEntry[parent] + (double) (now - entry);
      }
    }

    else if (task.getStatus () == Task::completed)
    {
      for (auto& parent : projects)
      {
        ++countCompleted[parent];

        time_t entry = strtol (task.get ("entry").c_str (), NULL, 10);
        time_t end   = strtol (task.get ("end").c_str (), NULL, 10);
        if (entry && end)
          sumEntry[parent] = sumEntry[parent] + (double) (end - entry);
      }
    }
  }

  // Create a table for output.
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string",            STRING_CMD_SUMMARY_PROJECT));
  view.add (Column::factory ("string.right",      STRING_CMD_SUMMARY_REMAINING));
  view.add (Column::factory ("string.right",      STRING_CMD_SUMMARY_AVG_AGE));
  view.add (Column::factory ("string.right",      STRING_CMD_SUMMARY_COMPLETE));
  view.add (Column::factory ("string.left_fixed", "0%                        100%"));

  Color bar_color;
  Color bg_color;
  if (context.color ())
  {
    bar_color = Color (context.config.get ("color.summary.bar"));
    bg_color  = Color (context.config.get ("color.summary.background"));

    Color label (context.config.get ("color.label"));
    view.colorHeader (label);
  }

  int barWidth = 30;
  std::vector <std::string> processed;
  for (auto& i : allProjects)
  {
    if (showAllProjects || countPending[i.first] > 0)
    {
      const std::vector <std::string> parents = extractParents (i.first);
      for (auto& parent : parents)
      {
        if (std::find (processed.begin (), processed.end (), parent)
           == processed.end ())
        {
          int row = view.addRow ();
          view.set (row, 0, indentProject (parent));
          processed.push_back (parent);
        }
      }

      int row = view.addRow ();
      view.set (row, 0, (i.first == ""
                          ? STRING_CMD_SUMMARY_NONE
                          : indentProject (i.first, "  ", '.')));

      view.set (row, 1, countPending[i.first]);
      if (counter[i.first])
        view.set (row, 2, ISO8601p ((int) (sumEntry[i.first] / (double)counter[i.first])).formatVague ());

      int c = countCompleted[i.first];
      int p = countPending[i.first];
      int completedBar = 0;
      if (c + p)
        completedBar = (c * barWidth) / (c + p);

      std::string bar;
      std::string subbar;
      if (context.color ())
      {
        bar += bar_color.colorize (std::string (           completedBar, ' '));
        bar += bg_color.colorize  (std::string (barWidth - completedBar, ' '));
      }
      else
      {
        bar += std::string (           completedBar, '=')
            +  std::string (barWidth - completedBar, ' ');
      }
      view.set (row, 4, bar);

      char percent[12] = "0%";
      if (c + p)
        sprintf (percent, "%d%%", 100 * c / (c + p));
      view.set (row, 3, percent);
      processed.push_back (i.first);
    }
  }

  std::stringstream out;
  if (view.rows ())
  {
    out << optionalBlankLine ()
        << view.render ()
        << optionalBlankLine ();

    out << format (STRING_CMD_PROJECTS_SUMMARY2, view.rows ()) << "\n";
  }
  else
  {
    out << STRING_CMD_PROJECTS_NO << "\n";
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
