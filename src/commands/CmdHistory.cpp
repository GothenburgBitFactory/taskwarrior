////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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
#include <CmdHistory.h>
#include <sstream>
#include <Context.h>
#include <Filter.h>
#include <Table.h>
#include <main.h>
#include <format.h>
#include <util.h>
#include <i18n.h>
#include <Datetime.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
template<class HistoryStrategy>
CmdHistoryBase<HistoryStrategy>::CmdHistoryBase ()
{
  _keyword               = HistoryStrategy::keyword;
  _usage                 = HistoryStrategy::usage;
  _description           = HistoryStrategy::description;

  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

template<class HistoryStrategy>
void CmdHistoryBase<HistoryStrategy>::outputGraphical (std::string &output)
{
  int widthOfBar = context.getWidth () - 15;   // 15 == strlen ("2008 September ")

  // Now build the view.
  Table view;
  if (context.config.getBoolean ("color"))
    view.forceColor ();
  view.width (context.getWidth ());

  HistoryStrategy::setupTableDates (view);

  view.add (STRING_CMD_GHISTORY_NUMBER); // Fixed.

  Color color_add    (context.config.get ("color.history.add"));
  Color color_done   (context.config.get ("color.history.done"));
  Color color_delete (context.config.get ("color.history.delete"));
  Color label        (context.config.get ("color.label"));

  view.colorHeader (label);

  // Determine the longest line, and the longest "added" line.
  int maxAddedLine = 0;
  int maxRemovedLine = 0;
  for (auto& i : groups)
  {
    if (completedGroup[i.first] + deletedGroup[i.first] > maxRemovedLine)
      maxRemovedLine = completedGroup[i.first] + deletedGroup[i.first];

    if (addedGroup[i.first] > maxAddedLine)
      maxAddedLine = addedGroup[i.first];
  }

  int maxLine = maxAddedLine + maxRemovedLine;
  if (maxLine > 0)
  {
    unsigned int leftOffset = (widthOfBar * maxAddedLine) / maxLine;

    int totalAdded     = 0;
    int totalCompleted = 0;
    int totalDeleted   = 0;

    time_t priorTime = 0;
    int row = 0;
    for (auto& i : groups)
    {
      row = view.addRow ();

      totalAdded     += addedGroup[i.first];
      totalCompleted += completedGroup[i.first];
      totalDeleted   += deletedGroup[i.first];

      HistoryStrategy::insertRowDate (view, row, i.first, priorTime);
      priorTime = i.first;

      unsigned int addedBar     = (widthOfBar *     addedGroup[i.first]) / maxLine;
      unsigned int completedBar = (widthOfBar * completedGroup[i.first]) / maxLine;
      unsigned int deletedBar   = (widthOfBar *   deletedGroup[i.first]) / maxLine;

      std::string bar = "";
      if (context.color ())
      {
        std::string aBar = "";
        if (addedGroup[i.first])
        {
          aBar = format (addedGroup[i.first]);
          while (aBar.length () < addedBar)
            aBar = ' ' + aBar;
        }

        std::string cBar = "";
        if (completedGroup[i.first])
        {
          cBar = format (completedGroup[i.first]);
          while (cBar.length () < completedBar)
            cBar = ' ' + cBar;
        }

        std::string dBar = "";
        if (deletedGroup[i.first])
        {
          dBar = format (deletedGroup[i.first]);
          while (dBar.length () < deletedBar)
            dBar = ' ' + dBar;
        }

        bar += std::string (leftOffset - aBar.length (), ' ');

        bar += color_add.colorize    (aBar);
        bar += color_done.colorize   (cBar);
        bar += color_delete.colorize (dBar);
      }
      else
      {
        std::string aBar = ""; while (aBar.length () < addedBar)     aBar += '+';
        std::string cBar = ""; while (cBar.length () < completedBar) cBar += 'X';
        std::string dBar = ""; while (dBar.length () < deletedBar)   dBar += '-';

        bar += std::string (leftOffset - aBar.length (), ' ');
        bar += aBar + cBar + dBar;
      }

      view.set (row, HistoryStrategy::dateFieldCount + 0, bar);
    }
  }

  std::stringstream out;
  if (view.rows ())
  {
    out << optionalBlankLine ()
        << view.render ()
        << '\n';

    if (context.color ())
      out << format (STRING_CMD_HISTORY_LEGEND,
                     color_add.colorize (STRING_CMD_HISTORY_ADDED),
                     color_done.colorize (STRING_CMD_HISTORY_COMP),
                     color_delete.colorize (STRING_CMD_HISTORY_DEL))
          << optionalBlankLine ()
          << '\n';
    else
      out << STRING_CMD_HISTORY_LEGEND_A
          << '\n';
  }
  else
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS);
    rc = 1;
  }

  output = out.str ();
}

template<class HistoryStrategy>
void CmdHistoryBase<HistoryStrategy>::outputTabular (std::string &output)
{
  Table view;
  if (context.config.getBoolean ("color"))
    view.forceColor ();
  view.width (context.getWidth ());

  HistoryStrategy::setupTableDates (view);

  view.add (STRING_CMD_HISTORY_ADDED, false);
  view.add (STRING_CMD_HISTORY_COMP,  false);
  view.add (STRING_CMD_HISTORY_DEL,   false);
  view.add (STRING_CMD_HISTORY_NET,   false);

  if (context.color ())
  {
    Color label (context.config.get ("color.label"));
    view.colorHeader (label);
  }

  int totalAdded     = 0;
  int totalCompleted = 0;
  int totalDeleted   = 0;

  int row = 0;
  time_t lastTime = 0;
  for (auto& i : groups)
  {
    row = view.addRow ();

    totalAdded     += addedGroup     [i.first];
    totalCompleted += completedGroup [i.first];
    totalDeleted   += deletedGroup   [i.first];

    HistoryStrategy::insertRowDate (view, row, i.first, lastTime);
    lastTime = i.first;

    int net = 0;

    if (addedGroup.find (i.first) != addedGroup.end ())
    {
      view.set (row, HistoryStrategy::dateFieldCount + 0, addedGroup[i.first]);
      net +=addedGroup[i.first];
    }

    if (completedGroup.find (i.first) != completedGroup.end ())
    {
      view.set (row, HistoryStrategy::dateFieldCount + 1, completedGroup[i.first]);
      net -= completedGroup[i.first];
    }

    if (deletedGroup.find (i.first) != deletedGroup.end ())
    {
      view.set (row, HistoryStrategy::dateFieldCount + 2, deletedGroup[i.first]);
      net -= deletedGroup[i.first];
    }

    Color net_color;
    if (context.color () && net)
      net_color = net > 0
                    ? Color (Color::red)
                    : Color (Color::green);

    view.set (row, HistoryStrategy::dateFieldCount + 3, net, net_color);
  }

  if (view.rows ())
  {
    row = view.addRow();
    view.set (row, 1, " ");
    row = view.addRow ();

    Color row_color;
    if (context.color ())
      row_color = Color (Color::nocolor, Color::nocolor, false, true, false);

    view.set (row, HistoryStrategy::dateFieldCount - 1, STRING_CMD_HISTORY_AVERAGE, row_color);
    view.set (row, HistoryStrategy::dateFieldCount + 0, totalAdded     / (view.rows () - 2), row_color);
    view.set (row, HistoryStrategy::dateFieldCount + 1, totalCompleted / (view.rows () - 2), row_color);
    view.set (row, HistoryStrategy::dateFieldCount + 2, totalDeleted   / (view.rows () - 2), row_color);
    view.set (row, HistoryStrategy::dateFieldCount + 3, (totalAdded - totalCompleted - totalDeleted) / (view.rows () - 2), row_color);
  }

  std::stringstream out;
  if (view.rows ())
    out << optionalBlankLine ()
        << view.render ()
        << '\n';
  else
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS);
    rc = 1;
  }

  output = out.str ();
}

template<class HistoryStrategy>
int CmdHistoryBase<HistoryStrategy>::execute (std::string& output)
{
  rc = 0;

  // TODO is this necessary?
  groups.clear ();
  addedGroup.clear ();
  deletedGroup.clear ();
  completedGroup.clear ();

  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  for (auto& task : filtered)
  {
    Datetime entry (task.get_date ("entry"));

    Datetime end;
    if (task.has ("end"))
      end = Datetime (task.get_date ("end"));

    time_t epoch = HistoryStrategy::getRelevantDate (entry).toEpoch ();
    groups[epoch] = 0;

    // Every task has an entry date, but exclude templates.
    if (task.getStatus () != Task::recurring)
      ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task.getStatus () == Task::deleted)
    {
      epoch = HistoryStrategy::getRelevantDate (end).toEpoch ();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task.getStatus () == Task::completed)
    {
      epoch = HistoryStrategy::getRelevantDate (end).toEpoch ();
      groups[epoch] = 0;
      ++completedGroup[epoch];
    }
  }

  // Now build the view.

  if (HistoryStrategy::graphical) {
    this->outputGraphical (output);
  } else {
    this->outputTabular (output);
  }

  return rc;
}

// Explicit instantiations, avoiding cpp-inclusion or implementation in header
template class CmdHistoryBase<MonthlyHistoryStrategy>;
template class CmdHistoryBase<AnnualHistoryStrategy>;
template class CmdHistoryBase<MonthlyGHistoryStrategy>;
template class CmdHistoryBase<AnnualGHistoryStrategy>;
