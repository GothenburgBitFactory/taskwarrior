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

////////////////////////////////////////////////////////////////////////////////
Table::Table ()
  : mRows (0)
  , mIntraPadding (1)
  , mDashedUnderline (false)
  , mTablePadding (0)
  , mTableWidth (0)
  , mSuppressWS (false)
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

  if (mSuppressWS)
  {
    std::string data2;
    if (mCommify.find (col) != mCommify.end ())
      data2 = commify (data);
    else
      data2 = data;

    clean (data2);
    // For multi-line cells, find the longest line.
    if (data2.find ("\n") != std::string::npos)
    {
      length = 0;
      std::vector <std::string> lines;
      split (lines, data2, "\n");
      for (unsigned int i = 0; i < lines.size (); ++i)
        if (lines[i].length () > length)
          length = lines[i].length ();
    }
    else
      length = data2.length ();

    mData.add (row, col, data2);
  }
  else
  {
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
  }

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

  std::string pad      = "";
  std::string intraPad = "";
  std::string preJust  = "";
  std::string attrOn   = "";
  std::string attrOff  = "";
  std::string postJust = "";

  for (int i = 0; i < padding; ++i)
    pad += " ";

  // Place the value within the available space - justify.
  int gap = width - data.length ();

  for (int i = 0; i < gap; ++i)
    postJust += " ";

  if (col < (signed) mColumns.size () - 1)
    for (int i = 0; i < getIntraPadding (); ++i)
      intraPad += " ";

  return c.colorize (pad + preJust + data + postJust + pad) + intraPad;
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

  std::string data     = "";
  for (int i = 0; i < width; ++i)
    data += '-';

  std::string pad      = "";
  std::string intraPad = "";
  std::string attrOn   = "";
  std::string attrOff  = "";

  for (int i = 0; i < padding; ++i)
    pad += " ";

  // Place the value within the available space - justify.
  if (col < (signed) mColumns.size () - 1)
    for (int i = 0; i < getIntraPadding (); ++i)
      intraPad += " ";

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

  std::string pad      = "";
  std::string intraPad = "";

  for (int i = 0; i < padding; ++i)
    pad += " ";

  if (col < (signed) mColumns.size () - 1)
    for (int i = 0; i < getIntraPadding (); ++i)
      intraPad += " ";

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
      for (int i = 0; i < gap; ++i)
        postJust += " ";

    else if (justification == right)
      for (int i = 0; i < gap; ++i)
        preJust += " ";

    else if (justification == center)
    {
      for (int i = 0; i < gap / 2; ++i)
        preJust += " ";

      for (size_t i = 0; i < gap - preJust.length (); ++i)
        postJust += " ";
    }

    lines.push_back (c.colorize (pad + preJust + chunks[chunk] + postJust + pad + intraPad));
  }

  // The blank is used to vertically pad cells that have blank lines.
  pad = "";
  for (int i = 0; i < width; ++i)
    pad += " ";

  blank = c.colorize (pad + intraPad);
}

////////////////////////////////////////////////////////////////////////////////
void Table::suppressWS ()
{
  mSuppressWS = true;
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
// Combsort11, with O(n log n) average, O(n log n) worst case performance.
//
// function combsort11(array input)
//     gap := input.size
//
//     loop until gap <= 1 and swaps = 0
//         if gap > 1
//             gap := gap / 1.3
//             if gap = 10 or gap = 9
//                 gap := 11
//             end if
//         end if
//
//         i := 0
//         swaps := 0
//
//         loop until i + gap >= input.size
//             if input[i] > input[i+gap]
//                 swap(input[i], input[i+gap])
//                 swaps := 1
//             end if
//             i := i + 1
//         end loop
//
//     end loop
// end function

#define SWAP \
   { \
     int temp = order[r]; \
     order[r] = order[r + gap]; \
     order[r + gap] = temp; \
     swaps = 1; \
   }
void Table::sort (std::vector <int>& order)
{
  int gap = order.size ();
  int swaps = 1;

  while (gap > 1 || swaps > 0)
  {
    if (gap > 1)
    {
      gap = (int) ((float)gap / 1.3);
      if (gap == 10 || gap == 9)
        gap = 11;
    }

    int r = 0;
    swaps = 0;

    while (r + gap < (int) order.size ())
    {
      bool keepScanning = true;
      for (size_t c = 0; keepScanning && c < mSortColumns.size (); ++c)
      {
        keepScanning = false;

        Grid::Cell* left  = mData.byRow (order[r],       mSortColumns[c]);
        Grid::Cell* right = mData.byRow (order[r + gap], mSortColumns[c]);

        // Data takes precedence over missing data.
        if (left == NULL && right != NULL)
        {
          SWAP
          break;
        }

        // No data - try comparing the next column.
        else if (left == NULL && right == NULL)
        {
          keepScanning = true;
        }

        // Identical data - try comparing the next column.
        else if (left && right && *left == *right)
        {
          keepScanning = true;
        }

        // Differing data - do a proper comparison.
        else if (left && right && *left != *right)
        {
          switch (mSortOrder[mSortColumns[c]])
          {
          case ascendingNumeric:
            if ((float)*left > (float)*right)
              SWAP
            break;

          case descendingNumeric:
            if ((float)*left < (float)*right)
              SWAP
            break;

          case ascendingCharacter:
            if ((std::string)*left > (std::string)*right)
              SWAP
            break;

          case descendingCharacter:
            if ((std::string)*left < (std::string)*right)
              SWAP
            break;

          case ascendingDate:
            {
              if ((std::string)*left != "" && (std::string)*right == "")
                break;

              else if ((std::string)*left == "" && (std::string)*right != "")
                SWAP

              else
              {
                Date dl ((std::string)*left, mDateFormat);
                Date dr ((std::string)*right, mDateFormat);
                if (dl > dr)
                  SWAP
              }
            }
            break;

          case descendingDate:
            {
              if ((std::string)*left != "" && (std::string)*right == "")
                break;

              else if ((std::string)*left == "" && (std::string)*right != "")
                SWAP

              else
              {
                Date dl ((std::string)*left, mDateFormat);
                Date dr ((std::string)*right, mDateFormat);
                if (dl < dr)
                  SWAP
              }
            }
            break;

          case ascendingDueDate:
            {
              if ((std::string)*left != "" && (std::string)*right == "")
                break;

              else if ((std::string)*left == "" && (std::string)*right != "")
                SWAP

              else
              {
                std::string format = context.config.get ("report." + mReportName + ".dateformat");
                if (format == "")
                  format = context.config.get ("dateformat.report");
                if (format == "")
                  format = context.config.get ("dateformat");

                Date dl ((std::string)*left,  format);
                Date dr ((std::string)*right, format);
                if (dl > dr)
                  SWAP
              }
            }
            break;

          case descendingDueDate:
            {
              if ((std::string)*left != "" && (std::string)*right == "")
                break;

              else if ((std::string)*left == "" && (std::string)*right != "")
                SWAP

              else
              {
                std::string format = context.config.get ("report." + mReportName + ".dateformat");
                if (format == "")
                  format = context.config.get ("dateformat.report");
                if (format == "")
                  format = context.config.get ("dateformat");

                Date dl ((std::string)*left,  format);
                Date dr ((std::string)*right, format);
                if (dl < dr)
                  SWAP
              }
            }
            break;

          case ascendingPriority:
            if (((std::string)*left == ""  && (std::string)*right  != "")  ||
                ((std::string)*left == "M" && (std::string)*right  == "L") ||
                ((std::string)*left == "H" && ((std::string)*right == "L"  || (std::string)*right == "M")))
              SWAP
            break;

          case descendingPriority:
            if (((std::string)*left == ""  && (std::string)*right  != "")                                 ||
                ((std::string)*left == "L" && ((std::string)*right == "M" || (std::string)*right == "H")) ||
                ((std::string)*left == "M" && (std::string)*right  == "H"))
              SWAP
            break;

          case ascendingPeriod:
            if ((std::string)*left == "" && (std::string)*right != "")
              break;
            else if ((std::string)*left != "" && (std::string)*right == "")
              SWAP
            else if (Duration ((std::string)*left) > Duration ((std::string)*right))
              SWAP
            break;

          case descendingPeriod:
            if ((std::string)*left != "" && (std::string)*right == "")
              break;
            else if ((std::string)*left == "" && (std::string)*right != "")
              SWAP
            else if (Duration ((std::string)*left) < Duration ((std::string)*right))
              SWAP
            break;
          }
        }
      }

      ++r;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Table::clean (std::string& value)
{
  size_t start = 0;
  size_t pos;
  while ((pos = value.find ('\t', start)) != std::string::npos)
  {
    value.replace (pos, 1, " ");
    start = pos;  // Not pos + 1, because we have a destructive operation, and
                  // this is ultimately safer.
  }

  while ((pos = value.find ('\r', start)) != std::string::npos)
  {
    value.replace (pos, 1, " ");
    start = pos;
  }

  while ((pos = value.find ('\n', start)) != std::string::npos)
  {
    value.replace (pos, 1, " ");
    start = pos;
  }
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
    sort (order);

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
