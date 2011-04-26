////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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

#include <vector>
#include <string>
#include <Task.h>

static std::vector <Task>* data = NULL;

////////////////////////////////////////////////////////////////////////////////
void view_sort (
  std::vector <Task>& data)
{
}

////////////////////////////////////////////////////////////////////////////////
// Re-implementation, using direct Task access instead of data copies that
// require re-parsing.
bool sort_compare (int left, int right)
{
}

////////////////////////////////////////////////////////////////////////////////
// Essentially a static implementation of a dynamic operator<.
bool sort_compare (int left, int right)
{
  for (size_t c = 0; c < table->mSortColumns.size (); ++c)
  {
    int column = table->mSortColumns[c];
    Table::order sort_type = table->mSortOrder[column];

    Grid::Cell* cell_left  = table->mData.byRow (left,  column);
    Grid::Cell* cell_right = table->mData.byRow (right, column);

    // Equally NULL - next column.
    if (cell_left == NULL && cell_right == NULL)
      continue;

    // Equal - next column
    if (cell_left && cell_right && *cell_left == *cell_right)
      continue;

    // Note: Table::ascendingDueDate is not represented here because it is not
    //       possible to have a NULL due date, only a blank "".

    // nothing < something.
    if (cell_left == NULL && cell_right != NULL)
      return (sort_type == Table::ascendingNumeric   ||
              sort_type == Table::ascendingCharacter ||
              sort_type == Table::ascendingPriority  ||
              sort_type == Table::ascendingDate      ||
              sort_type == Table::ascendingPeriod) ? true : false;

    // something > nothing.
    if (cell_left != NULL && cell_right == NULL)
      return (sort_type == Table::ascendingNumeric   ||
              sort_type == Table::ascendingCharacter ||
              sort_type == Table::ascendingPriority  ||
              sort_type == Table::ascendingDate      ||
              sort_type == Table::ascendingPeriod) ? false : true;

    // Differing data - do a proper comparison.
    if (cell_left && cell_right)
    {
      switch (sort_type)
      {
      case Table::ascendingNumeric:
        return (float)*cell_left < (float)*cell_right ? true : false;
        break;

      case Table::descendingNumeric:
        return (float)*cell_left < (float)*cell_right ? false : true;
        break;

      case Table::ascendingCharacter:
        return (std::string)*cell_left < (std::string)*cell_right ? true : false;
        break;

      case Table::descendingCharacter:
        return (std::string)*cell_left < (std::string)*cell_right ? false : true;
        break;

      case Table::ascendingDate:
        {
          // something > nothing.
          if ((std::string)*cell_left != "" && (std::string)*cell_right == "")
            return false;

          // nothing < something.
          else if ((std::string)*cell_left == "" && (std::string)*cell_right != "")
            return true;

          else
          {
            Date dl ((std::string)*cell_left, table->mDateFormat);
            Date dr ((std::string)*cell_right, table->mDateFormat);
            return dl < dr ? true : false;
          }
        }
        break;

      case Table::descendingDate:
        {
          // something > nothing.
          if ((std::string)*cell_left != "" && (std::string)*cell_right == "")
            return true;

          // nothing < something.
          else if ((std::string)*cell_left == "" && (std::string)*cell_right != "")
            return false;

          else
          {
            Date dl ((std::string)*cell_left, table->mDateFormat);
            Date dr ((std::string)*cell_right, table->mDateFormat);
            return dl < dr ? false : true;
          }
        }
        break;

      case Table::ascendingDueDate:
        {
          // something > nothing.
          if ((std::string)*cell_left != "" && (std::string)*cell_right == "")
            return true;

          // nothing < something.
          else if ((std::string)*cell_left == "" && (std::string)*cell_right != "")
            return false;

          else
          {
            std::string format = context.config.get ("report." + table->mReportName + ".dateformat");
            if (format == "")
              format = context.config.get ("dateformat.report");
            if (format == "")
              format = context.config.get ("dateformat");

            Date dl ((std::string)*cell_left,  format);
            Date dr ((std::string)*cell_right, format);
            return dl < dr ? true : false;
          }
        }
        break;

      case Table::descendingDueDate:
        {
          // something > nothing.
          if ((std::string)*cell_left != "" && (std::string)*cell_right == "")
            return true;

          // nothing < something.
          else if ((std::string)*cell_left == "" && (std::string)*cell_right != "")
            return false;

          else
          {
            std::string format = context.config.get ("report." + table->mReportName + ".dateformat");
            if (format == "")
              format = context.config.get ("dateformat.report");
            if (format == "")
              format = context.config.get ("dateformat");

            Date dl ((std::string)*cell_left,  format);
            Date dr ((std::string)*cell_right, format);
            return dl < dr ? false : true;
          }
        }
        break;

      case Table::ascendingPriority:
         if (((std::string)*cell_left == ""  && (std::string)*cell_right  != "")                                      ||
            ((std::string)*cell_left  == "L" && ((std::string)*cell_right == "M" || (std::string)*cell_right == "H")) ||
            ((std::string)*cell_left  == "M" && (std::string)*cell_right  == "H"))
          return true;
        else
          return false;
        break;

      case Table::descendingPriority:
        if (((std::string)*cell_left != ""  && (std::string)*cell_right  == "")  ||
            ((std::string)*cell_left == "M" && (std::string)*cell_right  == "L") ||
            ((std::string)*cell_left == "H" && ((std::string)*cell_right == "L"  || (std::string)*cell_right == "M")))
         return true;
        else
          return false;
        break;

      case Table::ascendingPeriod:
        if ((std::string)*cell_left == "" && (std::string)*cell_right != "")
          return true;
        else if ((std::string)*cell_left != "" && (std::string)*cell_right == "")
          return false;
        else
          return Duration ((std::string)*cell_left) < Duration ((std::string)*cell_right) ? true : false;
        break;

      case Table::descendingPeriod:
        if ((std::string)*cell_left != "" && (std::string)*cell_right == "")
          return false;
        else if ((std::string)*cell_left == "" && (std::string)*cell_right != "")
          return true;
        else
          return Duration ((std::string)*cell_left) < Duration ((std::string)*cell_right) ? false : true;
        break;
      }
    }
  }

  return false;
}
