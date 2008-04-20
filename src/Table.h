////////////////////////////////////////////////////////////////////////////////
// Copyright 2006 - 2008, Paul Beckingham.
// All rights reserved.
//
// TODO Implement height
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_TABLE
#define INCLUDED_TABLE

#include <map>
#include <vector>
#include <string>
#include "color.h"

class Table
{
public:
  enum just   {left, center, right};
  enum order  {ascendingNumeric, ascendingCharacter, ascendingPriority,
               ascendingDate, descendingNumeric, descendingCharacter,
               descendingPriority, descendingDate};
  enum sizing {minimum = -1, flexible = 0};

           Table ();
  virtual ~Table ();

           void setTableColor (Text::color, Text::color);
           void setTableFg (Text::color);
           void setTableBg (Text::color);
           void setTablePadding (int);
           void setTableIntraPadding (int);
           void setTableWidth (int);

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

           int rowCount ();
           int columnCount ();
           const std::string render ();

private:
           std::string getCell (int, int);
           Text::color getFg (int, int);
           Text::color getHeaderFg (int);
           Text::color getBg (int, int);
           Text::color getHeaderBg (int);
           Text::attr getHeaderUnderline (int);
           int getPadding (int);
           int getIntraPadding ();
           void calculateColumnWidths ();
           just getJustification (int, int);
           just getHeaderJustification (int);
           const std::string formatHeader (int, int, int);
           const std::string formatCell (int, int, int, int);
           void formatCell (int, int, int, int, std::vector <std::string>&, std::string&);
           void optimize (std::string&);
           void sort (std::vector <int>&);
           void clean (std::string&);

private:
  std::vector <std::string> mColumns;
  int mRows;
  int mIntraPadding;
  std::map <std::string, Text::color> mFg;
  std::map <std::string, Text::color> mBg;
  std::map <std::string, Text::attr> mUnderline;

  // Padding...
  int mTablePadding;
  std::vector <int> mColumnPadding;

  // Width...
  int mTableWidth;
  std::vector <int> mSpecifiedWidth;
  std::vector <int> mMaxDataWidth;
  std::vector <int> mCalculatedWidth;

  std::map <std::string, just> mJustification;
  std::map <int, bool> mCommify;
  std::map <std::string, std::string> mData;
  std::vector <int> mSortColumns;
  std::map <int, order> mSortOrder;

  // Misc...
  bool mSuppressWS;
};

#endif

////////////////////////////////////////////////////////////////////////////////
