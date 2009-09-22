////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#ifndef INCLUDED_TABLE
#define INCLUDED_TABLE

#include <map>
#include <vector>
#include <string>
#include "Color.h"
#include "Grid.h"

class Table
{
public:
  enum just   {left, center, right};
  enum order  {ascendingNumeric,
               ascendingCharacter,
               ascendingPriority,
               ascendingDate,
               ascendingPeriod,
               descendingNumeric,
               descendingCharacter,
               descendingPriority,
               descendingDate,
               descendingPeriod};
  enum sizing {minimum = -1, flexible = 0};

           Table ();
  virtual ~Table ();

           Table (const Table&);
           Table& operator= (const Table&);

           void setTableColor (Text::color, Text::color);
           void setTableFg (Text::color);
           void setTableBg (Text::color);
           void setTablePadding (int);
           void setTableIntraPadding (int);
           void setTableWidth (int);
           void setTableDashedUnderline ();

           int addColumn (const std::string&);
           void setColumnColor (int, Text::color, Text::color);
           void setColumnFg (int, Text::color);
           void setColumnBg (int, Text::color);
           void setColumnUnderline (int);
           void setColumnPadding (int, int);
           void setColumnWidth (int, int);
           void setColumnWidth (int, sizing);
           void setColumnJustification (int, just);
           void setColumnCommify (int);
           void sortOn (int, order);

           int addRow ();
           void setRowColor (int, Text::color, Text::color);
           void setRowFg (int, Text::color);
           void setRowBg (int, Text::color);

           void addCell (int, int, const std::string&);
           void addCell (int, int, char);
           void addCell (int, int, int);
           void addCell (int, int, float);
           void addCell (int, int, double);
           void setCellColor (int, int, Text::color, Text::color);
           void setCellFg (int, int, Text::color);
           void setCellBg (int, int, Text::color);

           void suppressWS ();
           void setDateFormat (const std::string&);

           int rowCount ();
           int columnCount ();
           const std::string render (int maximum = 0);

private:
           std::string getCell (const int, const int);
           Text::color getFg (const int, const int);
           Text::color getHeaderFg (const int);
           Text::color getBg (const int, const int);
           Text::color getHeaderBg (const int);
           Text::color getHeaderUnderline (const int);
           int getPadding (const int);
           int getIntraPadding ();
           void calculateColumnWidths ();
           just getJustification (const int, const int);
           just getHeaderJustification (const int);
           const std::string formatHeader (const int, const int, const int);
           const std::string formatHeaderDashedUnderline (const int, const int, const int);
           void formatCell (const int, const int, const int, const int, std::vector <std::string>&, std::string&);
           void sort (std::vector <int>&);
           void clean (std::string&);
           void optimize (std::string&) const;

private:
  std::vector <std::string> mColumns;
  int mRows;
  int mIntraPadding;
  std::map <std::string, std::string> mFg;
  std::map <std::string, std::string> mBg;
  std::map <std::string, std::string> mUnderline;
  bool mDashedUnderline;

  // Padding...
  int mTablePadding;
  std::vector <int> mColumnPadding;

  // Width...
  int mTableWidth;
  std::vector <int> mSpecifiedWidth;
  std::vector <int> mMaxDataWidth;
  std::vector <int> mCalculatedWidth;

  std::map <int, just> mJustification;
  std::map <int, bool> mCommify;
  Grid mData;
  std::vector <int> mSortColumns;
  std::map <int, order> mSortOrder;

  // Misc...
  bool mSuppressWS;
  std::string mDateFormat;
};

#endif

////////////////////////////////////////////////////////////////////////////////
