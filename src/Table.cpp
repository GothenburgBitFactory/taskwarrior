////////////////////////////////////////////////////////////////////////////////
// Copyright 2006 - 2008, Paul Beckingham.
// All rights reserved.
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
#include <Table.h>
#include <Date.h>
#include <library.h>
#include <stlmacros.h>

////////////////////////////////////////////////////////////////////////////////
Table::Table ()
  : mRows (0)
  , mIntraPadding (1)
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
void Table::setTableColor (Text::color fg, Text::color bg)
{
  mFg["table"] = fg;
  mBg["table"] = bg;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setTableFg (Text::color c)
{
  mFg["table"] = c;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setTableBg (Text::color c)
{
  mBg["table"] = c;
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
int Table::addColumn (const std::string& col)
{
  mSpecifiedWidth.push_back (minimum);
  mMaxDataWidth.push_back (col.length ());
  mCalculatedWidth.push_back (0);
  mColumnPadding.push_back (0);

  mColumns.push_back (col);
  return mColumns.size () - 1;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnColor (int column, Text::color fg, Text::color bg)
{
  char id[12];
  sprintf (id, "col:%d", column);
  mFg[id] = fg;
  mBg[id] = bg;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnFg (int column, Text::color c)
{
  char id[12];
  sprintf (id, "col:%d", column);
  mFg[id] = c;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnBg (int column, Text::color c)
{
  char id[12];
  sprintf (id, "col:%d", column);
  mBg[id] = c;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setColumnUnderline (int column)
{
  char id[12];
  sprintf (id, "col:%d", column);
  mUnderline[id] = Text::underline;
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
void Table::setRowColor (int row, Text::color fg, Text::color bg)
{
  char id[12];
  sprintf (id, "row:%d", row);
  mFg[id] = fg;
  mBg[id] = bg;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setRowFg (int row, Text::color c)
{
  char id[12];
  sprintf (id, "row:%d", row);
  mFg[id] = c;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setRowBg (int row, Text::color c)
{
  char id[12];
  sprintf (id, "row:%d", row);
  mBg[id] = c;
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (int row, int col, const std::string& data)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);

  int length = 0;

  if (mSuppressWS)
  {
    std::string data2;
    if (mCommify.find (col) != mCommify.end ())
      data2 = commify (data);
    else
      data2 = data;

    clean (data2);
    length = data2.length ();
    mData[id] = data2;
  }
  else
  {
    if (mCommify.find (col) != mCommify.end ())
      mData[id] = commify (data);
    else
      mData[id] = data;

    length = data.length ();
  }

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], length);
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (int row, int col, char data)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);

  char value[2];
  sprintf (value, "%c", data);

  mData[id] = value;

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], (signed) ::strlen (value));
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (int row, int col, int data)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);

  char value[12];
  sprintf (value, "%d", data);

  if (mCommify.find (col) != mCommify.end ())
    mData[id] = commify (value);
  else
    mData[id] = value;

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], (signed) ::strlen (value));
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (int row, int col, float data)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);

  char value[24];
  sprintf (value, "%.2f", data);

  if (mCommify.find (col) != mCommify.end ())
    mData[id] = commify (value);
  else
    mData[id] = value;

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], (signed) ::strlen (value));
}

////////////////////////////////////////////////////////////////////////////////
void Table::addCell (int row, int col, double data)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);

  char value[24];
  sprintf (value, "%.6f", data);

  if (mCommify.find (col) != mCommify.end ())
    mData[id] = commify (value);
  else
    mData[id] = value;

  // Automatically maintain max width.
  mMaxDataWidth[col] = max (mMaxDataWidth[col], (signed) ::strlen (value));
}

////////////////////////////////////////////////////////////////////////////////
void Table::setCellColor (int row, int col, Text::color fg, Text::color bg)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);
  mFg[id] = fg;
  mBg[id] = bg;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setCellFg (int row, int col, Text::color c)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);
  mFg[id] = c;
}

////////////////////////////////////////////////////////////////////////////////
void Table::setCellBg (int row, int col, Text::color c)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);
  mBg[id] = c;
}

////////////////////////////////////////////////////////////////////////////////
std::string Table::getCell (int row, int col)
{
  char id[24];
  sprintf (id, "cell:%d,%d", row, col);
  return mData[id];
}

////////////////////////////////////////////////////////////////////////////////
Text::color Table::getFg (int row, int col)
{
  char idCell[24];
  sprintf (idCell, "cell:%d,%d", row, col);
  if (mFg.find (idCell) != mFg.end ())
    return mFg[idCell];

  char idRow[12];
  sprintf (idRow, "row:%d", row);
  if (mFg.find (idRow) != mFg.end ())
    return mFg[idRow];

  char idCol[12];
  sprintf (idCol, "col:%d", col);
  if (mFg.find (idCol) != mFg.end ())
    return mFg[idCol];

  if (mFg.find ("table") != mFg.end ())
    return mFg["table"];

  return Text::nocolor;
}

////////////////////////////////////////////////////////////////////////////////
Text::color Table::getHeaderFg (int col)
{
  char idCol[12];
  sprintf (idCol,  "col:%d",          col);

  return mFg.find (idCol)   != mFg.end () ? mFg[idCol]
       : mFg.find ("table") != mFg.end () ? mFg["table"]
       : Text::nocolor;
}

////////////////////////////////////////////////////////////////////////////////
Text::color Table::getBg (int row, int col)
{
  char idCell[24];
  sprintf (idCell, "cell:%d,%d", row, col);
  if (mBg.find (idCell) != mBg.end ())
    return mBg[idCell];

  char idRow[12];
  sprintf (idRow, "row:%d", row);
  if (mBg.find (idRow) != mBg.end ())
    return mBg[idRow];

  char idCol[12];
  sprintf (idCol, "col:%d", col);
  if (mBg.find (idCol) != mBg.end ())
    return mBg[idCol];

  if (mBg.find ("table") != mBg.end ())
    return mBg["table"];

  return Text::nocolor;
}

////////////////////////////////////////////////////////////////////////////////
Text::color Table::getHeaderBg (int col)
{
  char idCol[12];
  sprintf (idCol,  "col:%d",          col);

  return mBg.find (idCol)   != mBg.end () ? mBg[idCol]
       : mBg.find ("table") != mBg.end () ? mBg["table"]
       : Text::nocolor;
}

////////////////////////////////////////////////////////////////////////////////
Text::attr Table::getHeaderUnderline (int col)
{
  char idCol[12];
  sprintf (idCol,  "col:%d",          col);

  return mUnderline.find (idCol) != mUnderline.end () ? Text::underline
       : Text::normal;
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
  for (unsigned int c = 0; c < mColumns.size (); ++c)
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
    for (unsigned int c = 0; c < mColumns.size (); ++c)
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
      for (unsigned int c = 0; c < mColumns.size (); ++c)
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
//    else
//      std::cout << "# insufficient room, considering only flexible columns." << std::endl;
  }

  // Try again, treating minimum columns as flexible.
//  std::cout << "# no flexible columns.  Now what?" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
Table::just Table::getJustification (int row, int col)
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

  Text::color fg        = getHeaderFg (col);
  Text::color bg        = getHeaderBg (col);
  std::string data      = mColumns[col];
  Text::attr decoration = getHeaderUnderline (col);

  std::string colorOn  = "";
  std::string pad      = "";
  std::string intraPad = "";
  std::string preJust  = "";
  std::string attrOn   = "";
  std::string attrOff  = "";
  std::string postJust = "";
  std::string colorOff = "";

  if (fg != Text::nocolor)
  {
    char c[12];
    sprintf (c, "\033[%dm", fg + 29);
    colorOn += c;
  }

  if (bg != Text::nocolor)
  {
    char c[12];
    sprintf (c, "\033[%dm", bg + 39);
    colorOn += c;
  }

  if (colorOn != "")
    colorOff += "\033[0m";

  if (decoration == Text::underline)
  {
    attrOn  = "\033[4m";
    attrOff = "\033[0m";
  }

  for (int i = 0; i < padding; ++i)
    pad += " ";

  // Place the value within the available space - justify.
  int gap = width - data.length ();

  for (int i = 0; i < gap; ++i)
    postJust += " ";

  if (col < (signed) mColumns.size () - 1)
    for (int i = 0; i < getIntraPadding (); ++i)
      intraPad += " ";

  return colorOn  +
         attrOn   +
         pad      +
         preJust  +
         data     +
         postJust +
         pad      +
         attrOff  +
         intraPad +
         colorOff;
}

////////////////////////////////////////////////////////////////////////////////
void Table::formatCell (
  int row,
  int col,
  int width,
  int padding,
  std::vector <std::string>& lines,
  std::string& blank)
{
  assert (width > 0);

  Text::color fg     = getFg (row, col);
  Text::color bg     = getBg (row, col);
  just justification = getJustification (row, col);
  std::string data   = getCell (row, col);

  std::string colorOn  = "";
  std::string pad      = "";
  std::string intraPad = "";
  std::string colorOff = "";

  if (fg != Text::nocolor)
  {
    char c[12];
    sprintf (c, "\033[%dm", fg + 29);
    colorOn += c;
  }

  if (bg != Text::nocolor)
  {
    char c[12];
    sprintf (c, "\033[%dm", bg + 39);
    colorOn += c;
  }

  if (fg != Text::nocolor || bg != Text::nocolor)
    colorOff += "\033[0m";

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
  for (unsigned int chunk = 0; chunk < chunks.size (); ++chunk)
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

      for (unsigned int i = 0; i < gap - preJust.length (); ++i)
        postJust += " ";
    }

    lines.push_back (
      colorOn       +
      pad           +
      preJust       +
      chunks[chunk] +
      postJust      +
      pad           +
      intraPad      +
      colorOff);
  }

  // The blank is used to vertically pad cells that have blank lines.
  pad = "";
  for (int i = 0; i < width; ++i)
    pad += " ";

  blank =
    colorOn  +
    pad      +
    intraPad +
    colorOff;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Table::formatCell (
  int row,
  int col,
  int width,
  int padding)
{
  assert (width > 0);

  Text::color fg     = getFg (row, col);
  Text::color bg     = getBg (row, col);
  just justification = getJustification (row, col);
  std::string data   = getCell (row, col);

  std::string colorOn  = "";
  std::string pad      = "";
  std::string intraPad = "";
  std::string preJust  = "";
  std::string postJust = "";
  std::string colorOff = "";

  if (fg != Text::nocolor)
  {
    char c[12];
    sprintf (c, "\033[%dm", fg + 29);
    colorOn += c;
  }

  if (bg != Text::nocolor)
  {
    char c[12];
    sprintf (c, "\033[%dm", bg + 39);
    colorOn += c;
  }

  if (fg != Text::nocolor || bg != Text::nocolor)
    colorOff += "\033[0m";

  for (int i = 0; i < padding; ++i)
    pad += " ";

  // Place the data within the available space - justify.
  int gap = width - data.length ();

  if (justification == left)
  {
    for (int i = 0; i < gap; ++i)
      postJust += " ";
  }
  else if (justification == right)
  {
    for (int i = 0; i < gap; ++i)
      preJust += " ";
  }
  else if (justification == center)
  {
    for (int i = 0; i < gap / 2; ++i)
      preJust += " ";

    for (unsigned int i = 0; i < gap - preJust.length (); ++i)
      postJust += " ";
  }

  if (col < (signed) mColumns.size () - 1)
    for (int i = 0; i < getIntraPadding (); ++i)
      intraPad += " ";

  return colorOn  +
         pad      +
         preJust  +
         data     +
         postJust +
         pad      +
         intraPad +
         colorOff;
}

////////////////////////////////////////////////////////////////////////////////
void Table::suppressWS ()
{
  mSuppressWS = true;
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
// Removes extraneous output characters, such as:
//   - spaces followed by a newline is collapsed to just a newline, if there is
//     no Bg color.
//   - removal of redundant color codes:
//       ^[[31mName^[[0m ^[[31mValue^[[0m -> ^[[31mName Value^[[0m
//
void Table::optimize (std::string& output)
{
/*
  TODO Unoptimized length.
  int start = output.length ();
*/

  // \s\n -> \n
  unsigned int i = 0;
  while ((i = output.find (" \n")) != std::string::npos)
  {
    output = output.substr (0, i) +
             output.substr (i + 1, std::string::npos);
  }

/*
  TODO This code displays the % reduction of the optimize function.
  std::cout << int ((100 * (start - output.length ()) / start))
            << "%" << std::endl;
*/
}

////////////////////////////////////////////////////////////////////////////////
// Combsort11, with O(n log n) average, O(n log n) worst case performance.
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

  while (gap > 1 || swaps != 0)
  {
    if (gap > 1)
    {
      gap = (int) ((float)gap / 1.3);
      if (gap == 10 or gap == 9)
        gap = 11;
    }

    int r = 0;
    swaps = 0;

    while (r + gap < (int) order.size ())
    {
      bool keepScanning = true;
      for (unsigned int c = 0; keepScanning && c < mSortColumns.size (); ++c)
      {
        keepScanning = false;

        char left[16];
        sprintf (left, "cell:%d,%d", order[r], mSortColumns[c]);

        char right[16];
        sprintf (right, "cell:%d,%d", order[r + gap], mSortColumns[c]);

        if (mData[left] != mData[right])
        {
          switch (mSortOrder[mSortColumns[c]])
          {
          case ascendingNumeric:
            if (::atoi (mData[left].c_str ()) > ::atoi (mData[right].c_str ()))
              SWAP
            break;

          case descendingNumeric:
            if (::atoi (mData[left].c_str ()) < ::atoi (mData[right].c_str ()))
              SWAP
            break;

          case ascendingCharacter:
            if (mData[left] > mData[right])
              SWAP
            break;

          case descendingCharacter:
            if (mData[left] < mData[right])
              SWAP
            break;

          case ascendingDate:
            {
              if (mData[left] != "" && mData[right] == "")
                break;

              else if (mData[left] == "" && mData[right] != "")
                SWAP

              else
              {
                Date dl (mData[left]);
                Date dr (mData[right]);
                if (dl > dr)
                  SWAP
              }
            }
            break;

          case descendingDate:
            {
              if (mData[left] != "" && mData[right] == "")
                break;

              else if (mData[left] == "" && mData[right] != "")
                SWAP

              else
              {
                Date dl (mData[left]);
                Date dr (mData[right]);
                if (dl < dr)
                  SWAP
              }
            }
            break;

          case ascendingPriority:
            if ((mData[left] == "" && mData[right] != "")                             ||
                (mData[left] == "M" && mData[right] == "L")                           ||
                (mData[left] == "H" && (mData[right] == "L" || mData[right] == "M")))
              SWAP
            break;

          case descendingPriority:
            if ((mData[left] == "" && mData[right] != "")                            ||
                (mData[left] == "L" && (mData[right] == "M" || mData[right] == "H")) ||
                (mData[left] == "M" && mData[right] == "H"))
              SWAP
            break;
          }

          break;
        }
        else
          keepScanning = true;
      }

      ++r;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Table::clean (std::string& value)
{
  unsigned int start = 0;
  unsigned int pos;
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
const std::string Table::render ()
{
  calculateColumnWidths ();

  // Print column headers in column order.
  std::string output;
  for (unsigned int col = 0; col < mColumns.size (); ++col)
    output += formatHeader (
                col,
                mCalculatedWidth[col],
                mColumnPadding[col]);

  output += "\n";

  // Determine row order, according to sort options.
  std::vector <int> order;
  for (int row = 0; row < mRows; ++row)
    order.push_back (row);

  // Only sort if necessary.
  if (mSortColumns.size ())
    sort (order);

  // Print all rows.
  for (int row = 0; row < mRows; ++row)
  {
    std::vector <std::vector <std::string> > columns;
    std::vector <std::string> blanks;

    unsigned int maxHeight = 0;
    for (unsigned int col = 0; col < mColumns.size (); ++col)
    {
      std::vector <std::string> lines;
      std::string blank;
      formatCell (
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
      for (unsigned int lines = 0; lines < maxHeight; ++lines)
      {
        for (unsigned int col = 0; col < mColumns.size (); ++col)
          if (lines < columns[col].size ())
            output += columns[col][lines];
          else
            output += blanks[col];

        output += "\n";
      }
    }
    else
      output += "\n";
  }

  // Eliminate redundant color codes.
  optimize (output);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
