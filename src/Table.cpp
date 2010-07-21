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
//
//
// Attributes                Table    Row Column   Cell
// ----------------------------------------------------
// foreground color              Y      Y      Y      Y
// background color              Y      Y      Y      Y
// padding                       Y      Y      Y      Y
// wrap                          Y      Y      Y      Y
// width                         Y      -      Y      Y
// height                        -      Y      -      Y
// justification                 Y      Y      Y      Y
//
// Precedence
//   If attributes conflict, the precedence order is:
//     cell
//     row
//     column
//     table
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string.h>
#include <assert.h>
#include "Table.h"
#include "Date.h"
#include "Duration.h"
#include "Timer.h"
#include "text.h"
#include "util.h"
#include "Context.h"

extern Context context;
Table* table = NULL;

////////////////////////////////////////////////////////////////////////////////
Table::Table ()
  : mRows (0)
  , mIntraPadding (1)
  , mDashedUnderline (false)
  , mTablePadding (0)
  , mTableWidth (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Table::~Table ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Table::setTableAlternateColor (const Color& c)
{
  alternate = c;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setTablePadding (int padding)
{
  mTablePadding = padding;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setTableIntraPadding (int padding)
{
  mIntraPadding = padding;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setTableWidth (int width)
{
  assert (width > 0);
  mTableWidth = width;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setTableDashedUnderline ()
{
  mDashedUnderline = true;
}

////////////////////////////////////////////////////////////////////////////////
int Table::addColumn (const std::string& col)
{
  mSpecifiedWidth.push_back (minimum);
  mMaxDataWidth.push_back (col == "" ? 1 : col.length ());
  mCalculatedWidth.push_back (0);
  mColumnPadding.push_back (0);

  mColumns.push_back (col == "" ? " " : col);
  return mColumns.size () - 1;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnUnderline (int column)
{
  char id[12];
  sprintf (id, "col:%d", column);
  mUnderline[id] = Color (Color::nocolor, Color::nocolor, true, false, false);
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnPadding (int column, int padding)
{
  mColumnPadding[column] = padding;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnWidth (int column, int width)
{
  assert (width > 0);
  mSpecifiedWidth[column] = width;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnWidth (int column, sizing s)
{
  mSpecifiedWidth[column] = (int) s;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnCommify (int column)
{
  mCommify[column] = true;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnJustification (int column, just j)
{
  mJustification[column] = j;
}

////////////////////////////////////////////////////////////////////////////////
void Table::sortOn (int column, order o)
{
  mSortColumns.push_back (column);
  mSortOrder[column] = o;
}

////////////////////////////////////////////////////////////////////////////////
int Table::addRow ()
{
  return mRows++;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setRowColor (const int row, const Color& c)
{
  char id[12];
  sprintf (id, "row:%d", row);
  mColor[id].blend (c);
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (const int row, const int col, const std::string& data)
{
  unsigned int length = 0;

  if (mCommify.find (col) != mCommify.end ())
    mData.add (row, col, commify (data));
  else
    mData.add (row, col, data);

  // For multi-line cells, find the longest line.
  if (data.find ("\n") != std::string::npos)
  {
    length = 0;
    std::vector <std::string> lines;
    split (lines, data, "\n");
    for (unsigned int i = 0; i < lines.size (); ++i)
      if (lines[i].length () > length)
        length = lines[i].length ();
  }
  else
    length = data.length ();

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], (int)length);
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (const int row, const int col, const char data)
{
  mData.add (row, col, data);

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], 1);
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (const int row, const int col, const int data)
{
  char value[12];
  sprintf (value, "%d", data);

  if (mCommify.find (col) != mCommify.end ())
    mData.add (row, col, commify (value));
  else
    mData.add (row, col, value);

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], (signed) strlen (value));
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (const int row, const int col, const float data)
{
  char value[24];
  sprintf (value, "%.2f", data);

  if (mCommify.find (col) != mCommify.end ())
    mData.add (row, col, commify (value));
  else
    mData.add (row, col, value);

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], (signed) strlen (value));
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (const int row, const int col, const double data)
{
  char value[24];
  sprintf (value, "%.6f", data);

  if (mCommify.find (col) != mCommify.end ())
    mData.add (row, col, commify (value));
  else
    mData.add (row, col, value);

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], (signed) strlen (value));
}

////////////////////////////////////////////////////////////////////////////////
void Table::setCellColor (const int row, const int col, const Color& c)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);
  mColor[id] = c;
}

////////////////////////////////////////////////////////////////////////////////
std::string Table::getCell (const int row, const int col)
{
  Grid::Cell* c = mData.byRow (row, col);
  if (c)
    return (std::string) *c;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
Color Table::getColor (const int index, const int row, const int col)
{
  // Color defaults to trivial.
  Color c;

  // For alternating rows, use Table::alternate.
  std::map <std::string, Color>::iterator i;
  char id[24];

  if (index % 2)
    c = alternate;

/*
  // TODO Obsolete - this is not used.  Consider removal.
  // Blend with a table color, if specified.
  if ((i = mColor.find ("table")) != mColor.end ())
    c.blend (i->second);

  // Blend with a column color, if specified.
  sprintf (id, "col:%d", col);
  if ((i = mColor.find (id)) != mColor.end ())
    c.blend (i->second);
*/

  // Blend with a row color, if specified.
  sprintf (id, "row:%d", row);
  if ((i = mColor.find (id)) != mColor.end ())
    c.blend (i->second);

  // Blend with a cell color, if specified.
  sprintf (id, "cell:%d,%d", row, col);
  if ((i = mColor.find (id)) != mColor.end ())
    c.blend (i->second);

  return c;
}

////////////////////////////////////////////////////////////////////////////////
Color Table::getHeaderUnderline (int col)
{
  char idCol[12];
  sprintf (idCol,  "col:%d",          col);

  return mUnderline.find (idCol) != mUnderline.end () ? mUnderline[idCol]
       : Color ();
}

////////////////////////////////////////////////////////////////////////////////
int Table::getIntraPadding ()
{
  return mIntraPadding;
}

////////////////////////////////////////////////////////////////////////////////
int Table::getPadding (int col)
{
  return max (mColumnPadding[col], mTablePadding);
}

////////////////////////////////////////////////////////////////////////////////
// Using mSpecifiedWidth, mMaxDataWidth, generate mCalculatedWidth.
void Table::calculateColumnWidths ()
{
  // Ideal case: either no table width is specified, or everything fits without
  // wrapping into mTableWidth.
  std::vector <int> ideal = mMaxDataWidth;
  int width = 0;
  int countFlexible = 0;
  for (size_t c = 0; c < mColumns.size (); ++c)
  {
    if (mSpecifiedWidth[c] == flexible)
      ++countFlexible;

    else if (mSpecifiedWidth[c] > 0)
      ideal[c] = mSpecifiedWidth[c];

    width += mColumnPadding[c] +
             ideal[c]          +
             mColumnPadding[c] +
             (c > 0 ? mIntraPadding : 0);
  }

  if (!mTableWidth || width < mTableWidth)
  {
    mCalculatedWidth = ideal;
    return;
  }

  // Try again, with available space divided among the flexible columns.
  if (countFlexible)
  {
    ideal = mMaxDataWidth;
    width = 0;
    for (size_t c = 0; c < mColumns.size (); ++c)
    {
      if (mSpecifiedWidth[c] > 0)
        ideal[c] = mSpecifiedWidth[c];
      else if (mSpecifiedWidth[c] == flexible)
      {
        ideal[c] = 0;
      }

      width += mColumnPadding[c] +
               ideal[c]          +
               mColumnPadding[c] +
               (c > 0 ? mIntraPadding : 0);
    }

    int available = mTableWidth - width;
    if (width < mTableWidth)   // if there is room to wiggle in
    {
      int shared = available / countFlexible;
      int remainder = available % countFlexible;

      int lastFlexible = mColumns.size () - 1;
      for (size_t c = 0; c < mColumns.size (); ++c)
      {
         if (mSpecifiedWidth[c] == flexible)
         {
           lastFlexible = c;
           ideal[c] += shared;
         }
      }

      // Remainder goes to last column.
      ideal[lastFlexible] += remainder;
      mCalculatedWidth = ideal;
      return;
    }
    else
    {
      // The fallback position is to assume no width was specified, and just
      // calculate widths accordingly.
      mTableWidth = 0;
      calculateColumnWidths ();
      return;
    }
  }

  // Try again, treating minimum columns as flexible.
//  std::cout << "# no flexible columns.  Now what?" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
Table::just Table::getJustification (const int row, const int col)
{
  if (mJustification.find (col) != mJustification.end ())
    return mJustification[col];

  return left;
}

////////////////////////////////////////////////////////////////////////////////
Table::just Table::getHeaderJustification (int col)
{
  return mJustification.find (col) != mJustification.end () ? mJustification[col]
       : left;
}

////////////////////////////////////////////////////////////////////////////////
// data            One    Data to be rendered
// width           8      Max data width for column/specified width
// padding         1      Extra padding around data
// intraPadding    0      Extra padding between columns only
// justification   right  Alignment withing padding
//
// Returns:
//      "    One "
//           One   data
//       ^      ^  padding
//              ^  intraPadding
//       ^^^^^^^^  width
//      ^        ^ fg/bg
//
const std::string Table::formatHeader (
  int col,
  int width,
  int padding)
{
  assert (width > 0);

  std::string data = mColumns[col];
  Color c = getHeaderUnderline (col);
  int gap = width - strippedLength (data);

  std::string pad = std::string (padding, ' ');

  // TODO When the following is replaced by:
  //      std::string postJust = std::string (gap, ' ');
  //      two unit tests fail.
  std::string postJust = "";
  for (int i = 0; i < gap; ++i)
    postJust += " ";

  std::string intraPad = "";
  if (col < (signed) mColumns.size () - 1)
    intraPad = std::string (getIntraPadding (), ' ');

  return c.colorize (pad + data + postJust + pad) + intraPad;
}

////////////////////////////////////////////////////////////////////////////////
// data            One    Data to be rendered
// width           8      Max data width for column/specified width
// padding         1      Extra padding around data
// intraPadding    0      Extra padding between columns only
// justification   right  Alignment withing padding
//
// Returns:
//      "------- "
//       -------   data
//       ^      ^  padding
//              ^  intraPadding
//       ^^^^^^^^  width
//      ^        ^ fg/bg
//
const std::string Table::formatHeaderDashedUnderline (
  int col,
  int width,
  int padding)
{
  assert (width > 0);

  Color c = getHeaderUnderline (col);

  std::string data = std::string (width, '-');
  std::string pad = std::string (padding, ' ');
  std::string intraPad = "";

  // Place the value within the available space - justify.
  if (col < (signed) mColumns.size () - 1)
    intraPad = std::string (getIntraPadding (), ' ');

  return c.colorize (pad + data + pad) + intraPad;
}

////////////////////////////////////////////////////////////////////////////////
void Table::formatCell (
  const int index,
  const int row,
  const int col,
  const int width,
  const int padding,
  std::vector <std::string>& lines,
  std::string& blank)
{
  assert (width > 0);

  Color c = getColor (index, row, col);
  just justification = getJustification (row, col);
  std::string data   = getCell (row, col);

  std::string pad = std::string (padding, ' ');
  std::string intraPad = "";

  if (col < (signed) mColumns.size () - 1)
    intraPad = std::string (getIntraPadding (), ' ');

  // Break the text into chunks of width characters.
  std::string preJust;
  std::string postJust;
  std::vector <std::string> chunks;
  wrapText (chunks, data, width);
  for (size_t chunk = 0; chunk < chunks.size (); ++chunk)
  {
    // Place the data within the available space - justify.
    int gap = width - chunks[chunk].length ();

    preJust = "";
    postJust = "";

    if (justification == left)
      postJust = std::string (gap, ' ');

    else if (justification == right)
      preJust = std::string (gap, ' ');

    else if (justification == center)
    {
      preJust = std::string (gap / 2, ' ');
      postJust = std::string (gap - preJust.length (), ' ');
    }

    lines.push_back (c.colorize (pad + preJust + chunks[chunk] + postJust + pad + intraPad));
  }

  // The blank is used to vertically pad cells that have blank lines.
  pad = std::string (width, ' ');

  blank = c.colorize (pad + intraPad);
}

////////////////////////////////////////////////////////////////////////////////
void Table::setDateFormat (const std::string& dateFormat)
{
  mDateFormat = dateFormat;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setReportName (const std::string& reportName)
{
  mReportName = reportName;
}

////////////////////////////////////////////////////////////////////////////////
int Table::rowCount ()
{
  return mRows;
}

////////////////////////////////////////////////////////////////////////////////
int Table::columnCount ()
{
  return mColumns.size ();
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

    // nothing < something.
    if (cell_left == NULL && cell_right != NULL)
      return (sort_type == Table::ascendingNumeric ||
              sort_type == Table::ascendingCharacter ||
              sort_type == Table::ascendingPriority ||
              sort_type == Table::ascendingDate ||
              sort_type == Table::ascendingDueDate ||
              sort_type == Table::ascendingPeriod) ? true : false;

    // something > nothing.
    if (cell_left != NULL && cell_right == NULL)
      return (sort_type == Table::ascendingNumeric ||
              sort_type == Table::ascendingCharacter ||
              sort_type == Table::ascendingPriority ||
              sort_type == Table::ascendingDate ||
              sort_type == Table::ascendingDueDate ||
              sort_type == Table::ascendingPeriod) ? false : true;

    // Equally NULL - next column.
    if (cell_left == NULL && cell_right == NULL)
      continue;

    // Equal - next column
    if (cell_left && cell_right && *cell_left == *cell_right)
      continue;

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
            return false;

          // nothing < something.
          else if ((std::string)*cell_left == "" && (std::string)*cell_right != "")
            return true;

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
        if (((std::string)*cell_left == ""  && (std::string)*cell_right  != "")  ||
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
        else if (Duration ((std::string)*cell_left) < Duration ((std::string)*cell_right))
          return true;
        break;

      case Table::descendingPeriod:
        if ((std::string)*cell_left != "" && (std::string)*cell_right == "")
          return false;
        else if ((std::string)*cell_left == "" && (std::string)*cell_right != "")
          return true;
        else if (Duration ((std::string)*cell_left) < Duration ((std::string)*cell_right))
          return false;
        break;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Table::render (int maxrows /* = 0 */, int maxlines /* = 0 */)
{
  Timer t ("Table::render");

  // May not exceed maxlines, if non-zero.
  int renderedlines = 0;

  calculateColumnWidths ();

  // Print column headers in column order.
  std::string output;
  std::string underline;
  for (size_t col = 0; col < mColumns.size (); ++col)
  {
    output += formatHeader (
                col,
                mCalculatedWidth[col],
                mColumnPadding[col]);

    if (mDashedUnderline)
      underline += formatHeaderDashedUnderline (
                     col,
                     mCalculatedWidth[col],
                     mColumnPadding[col]);
  }

  output += "\n";
  ++renderedlines;
  if (underline.length ())
  {
    output += underline + "\n";
    ++renderedlines;
  }

  // Determine row order, according to sort options.
  std::vector <int> order;
  for (int row = 0; row < mRows; ++row)
    order.push_back (row);

  // Only sort if necessary.
  if (mSortColumns.size ())
  {
    table = this;  // Substitute for 'this' in the static 'sort_compare'.
    std::sort (order.begin (), order.end (), sort_compare);
  }

  // If a non-zero maxrows is specified, then it limits the number of rows of
  // the table that are rendered.
  int limitrows = mRows;
  if (maxrows != 0)
    limitrows = min (maxrows, mRows);

  // If a non-zero maxlines is specified, then it limits the number of lines
  // of output from the table that are rendered.

  // Print all rows.
  for (int row = 0; row < limitrows; ++row)
  {
    std::vector <std::vector <std::string> > columns;
    std::vector <std::string> blanks;

    size_t maxHeight = 0;
    for (size_t col = 0; col < mColumns.size (); ++col)
    {
      std::vector <std::string> lines;
      std::string blank;
      formatCell (
        row,
        order[row],
        col,
        mCalculatedWidth[col],
        mColumnPadding[col],
        lines,
        blank);

      columns.push_back (lines);
      blanks.push_back (blank);

      maxHeight = max (maxHeight, columns[col].size ());
    }

    if (maxHeight)
    {
      for (size_t lines = 0; lines < maxHeight; ++lines)
      {
        for (size_t col = 0; col < mColumns.size (); ++col)
          if (lines < columns[col].size ())
            output += columns[col][lines];
          else
            output += blanks[col];

        // Trim right.
        output.erase (output.find_last_not_of (" ") + 1);
        output += "\n";

        ++renderedlines;

        if (maxlines != 0 && renderedlines >= maxlines)
          break;
      }
    }
    else
    {
      // Trim right.
      output.erase (output.find_last_not_of (" ") + 1);
      output += "\n";
    }

    if (maxlines != 0 && renderedlines >= maxlines)
      break;
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
