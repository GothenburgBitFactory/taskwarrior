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
// Grid implements a sparse 2D array of Cell objects.  Grid makes every effort
// to perform well on cell insertion and retrieval.  A Cell is a variant type,
// capable of storing char, bool, int, float, double and std::string types.
//
// Every cell is accessible from both mColumns and mRows.  This allows the
// client code to specify which method is used, because there will be a
// performance penalty with one of the methods, depending on the layout of
// cells within the grid.
//
// mColumns, like mRows, is a vector of a vector of Cell*.
//
//                     mColumns
//                     [0..n]
//                    +---+---+-----------+---+
//                    | 0 | 1 |           | n |
//                    +---+---+-----------+---+
//                          |               |
//                          v               |
//         +---+      .   +---+           . | .
//  mRows  | 0 | -------> | x |             v
//  [0..1] +---+      .   +---+           +---+
//         | 1 | -------> | y | --------> | z |
//         +---+      .   +---+           +---+
//         |   |
//         |   |
//         |   |
//         +---+      .   .   .           .   .
//         | n |
//         +---+      .   .   .           .   .
//
//
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include <Grid.h>

////////////////////////////////////////////////////////////////////////////////
Grid::Grid ()
{
}

////////////////////////////////////////////////////////////////////////////////
// The cells are deleted via their mRows reference, not by their mColumns
// reference.  This is because the cells are doubly-linked, and so the
// convention is that rows own cells, columns merely own pointers.
Grid::~Grid ()
{
  std::vector < std::vector <Cell*>* >::iterator row;
  std::vector <Cell*>::iterator col;
  for (row = mRows.begin (); row != mRows.end (); ++row)
    if (*row)
      for (col = (*row)->begin (); col != (*row)->end (); ++col)
        if (*col)
          delete *col;

  std::vector < std::vector <Cell*>* >::iterator it;
  for (it = mRows.begin (); it != mRows.end (); ++it)
    if (*it)
      delete *it;

  for (it = mColumns.begin (); it != mColumns.end (); ++it)
    if (*it)
      delete *it;
}

////////////////////////////////////////////////////////////////////////////////
void Grid::add (
  const unsigned int row,
  const unsigned int col,
  const bool value)
{
  expandGrid (row, col);
  insertCell (row, col, new Cell (value));
}

void Grid::add (
  const unsigned int row,
  const unsigned int col,
  const char value)
{
  expandGrid (row, col);
  insertCell (row, col, new Cell (value));
}

void Grid::add (
  const unsigned int row,
  const unsigned int col,
  const int value)
{
  expandGrid (row, col);
  insertCell (row, col, new Cell (value));
}

void Grid::add (
  const unsigned int row,
  const unsigned int col,
  const float value)
{
  expandGrid (row, col);
  insertCell (row, col, new Cell (value));
}

void Grid::add (
  const unsigned int row,
  const unsigned int col,
  const double value)
{
  expandGrid (row, col);
  insertCell (row, col, new Cell (value));
}

void Grid::add (
  const unsigned int row,
  const unsigned int col,
  const char* value)
{
  expandGrid (row, col);
  insertCell (row, col, new Cell (std::string (value)));
}

void Grid::add (
  const unsigned int row,
  const unsigned int col,
  const std::string& value)
{
  expandGrid (row, col);
  insertCell (row, col, new Cell (value));
}

////////////////////////////////////////////////////////////////////////////////
unsigned int Grid::width () const
{
  return mColumns.size ();
}

unsigned int Grid::height () const
{
  return mRows.size ();
}

////////////////////////////////////////////////////////////////////////////////
Grid::Cell* Grid::byRow (const unsigned int row, const unsigned int col) const
{
  if (row < mRows.size () &&
      mRows[row] != NULL   &&
      col < mRows[row]->size ())
    return (*mRows[row])[col];

  return NULL;
}

Grid::Cell* Grid::byColumn (const unsigned int row, const unsigned int col) const
{
  if (col < mColumns.size () &&
      mColumns[col] != NULL   &&
      row < mColumns[col]->size ())
    return (*mColumns[col])[row];

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void Grid::expandGrid (const unsigned int row, const unsigned int col)
{

  // If the new row is outside the bounds of the current grid, add blank rows to
  // pad, then a new row vector.
  if (row >= mRows.size ())
  {
    for (unsigned int r = mRows.size (); r <= row; ++r)
      if (r < row)
        mRows.push_back (NULL);
      else
        mRows.push_back (new std::vector <Cell*>);
  }
  // If the new row is within the bounds of the current grid, ensure that the
  // row points to a vector of cells.
  else if (mRows[row] == NULL)
    mRows[row] = new std::vector <Cell*>;

  if (col >= mRows[row]->size ())
    for (unsigned int c = mRows[row]->size (); c <= col; ++c)
      mRows[row]->push_back (NULL);

  // If the new col is outside the bounds of the current grid, add blank cols to
  // pad, then a new col vector.
  if (col >= mColumns.size ())
  {
    for (unsigned int c = mColumns.size (); c <= col; ++c)
      if (c < col)
        mColumns.push_back (NULL);
      else
        mColumns.push_back (new std::vector <Cell*>);
  }
  // If the new col is within the bounds of the current grid, ensure that the
  // col points to a vector of cells.
  else if (mColumns[col] == NULL)
    mColumns[col] = new std::vector <Cell*>;

  if (row >= mColumns[col]->size ())
    for (unsigned int r = mColumns[col]->size (); r <= row; ++r)
      mColumns[col]->push_back (NULL);
}

////////////////////////////////////////////////////////////////////////////////
void Grid::insertCell (
  const unsigned int row,
  const unsigned int col,
  Cell* cell)
{
  // Delete any existing cell, because cells are owned by rows, not columns.
  if ((*mRows[row])[col] != NULL)
    delete (*mRows[row])[col];

  (*mRows[row])[col]    = cell;
  (*mColumns[col])[row] = cell;
}

////////////////////////////////////////////////////////////////////////////////
Grid::Cell::Cell (const bool value)
: mType (CELL_BOOL)
, mBool (value)
{
}

Grid::Cell::Cell (const char value)
: mType (CELL_CHAR)
, mChar (value)
{
}

Grid::Cell::Cell (const int value)
: mType (CELL_INT)
, mInt (value)
{
}

Grid::Cell::Cell (const float value)
: mType (CELL_FLOAT)
, mFloat (value)
{
}

Grid::Cell::Cell (const double value)
: mType (CELL_DOUBLE)
, mDouble (value)
{
}

Grid::Cell::Cell (const std::string& value)
: mType (CELL_STRING)
, mString (value)
{
}

////////////////////////////////////////////////////////////////////////////////
// These cast operators make a best approximation to an appropriate rendering,
// given the format change.
Grid::Cell::operator bool () const
{
  switch (mType)
  {
  case CELL_BOOL:   return mBool;
  case CELL_CHAR:   return mChar != '\0' &&
                           mChar != ' '  &&
                           mChar != '0';
  case CELL_INT:    return mInt != 0;
  case CELL_FLOAT:  return mFloat != 0.0;
  case CELL_DOUBLE: return mDouble != 0.0;
  case CELL_STRING: return mString.length () > 0;
  }

  return false;
}

Grid::Cell::operator char () const
{
  switch (mType)
  {
  case CELL_BOOL:   return mBool ? 'Y' : 'N'; // TODO i18n
  case CELL_CHAR:   return mChar;
  case CELL_INT:    return (char) mInt;
  case CELL_FLOAT:  return (char) (int) mFloat;
  case CELL_DOUBLE: return (char) (int) mDouble;
  case CELL_STRING: return mString[0];
  }

  return '\0';
}

Grid::Cell::operator int () const
{
  switch (mType)
  {
  case CELL_BOOL:   return mBool ? 1 : 0;
  case CELL_CHAR:   return (int) mChar;
  case CELL_INT:    return mInt;
  case CELL_FLOAT:  return (int) mFloat;
  case CELL_DOUBLE: return (int) mDouble;
  case CELL_STRING: return atoi (mString.c_str ());
  }

  return 0;
}

Grid::Cell::operator float () const
{
  switch (mType)
  {
  case CELL_BOOL:   return mBool ? 1.0 : 0.0;
  case CELL_CHAR:   return (float) (int) mChar;
  case CELL_INT:    return (float) mInt;
  case CELL_FLOAT:  return mFloat;
  case CELL_DOUBLE: return (float) mDouble;
  case CELL_STRING: return (float) atof (mString.c_str ());
  }

  return 0.0;
}

Grid::Cell::operator double () const
{
  switch (mType)
  {
  case CELL_BOOL:   return mBool ? 1.0 : 0.0;
  case CELL_CHAR:   return (double) (int) mChar;
  case CELL_INT:    return (double) mInt;
  case CELL_FLOAT:  return (double) mFloat;
  case CELL_DOUBLE: return mDouble;
  case CELL_STRING: return (double) atof (mString.c_str ());
  }

  return 0.0;
}

Grid::Cell::operator std::string () const
{
  char s[64] = {0};

  switch (mType)
  {
  case CELL_BOOL:   return mBool ? "true" : "false"; // TODO i18n
  case CELL_CHAR:   sprintf (s, "%c", mChar);   return std::string (s);
  case CELL_INT:    sprintf (s, "%d", mInt);    return std::string (s);
  case CELL_FLOAT:  sprintf (s, "%f", mFloat);  return std::string (s);
  case CELL_DOUBLE: sprintf (s, "%f", mDouble); return std::string (s);
  case CELL_STRING:                             return mString;
  }

  return std::string ("");
}

////////////////////////////////////////////////////////////////////////////////
bool Grid::Cell::operator== (const Grid::Cell& rhs) const
{
  switch (mType)
  {
  case CELL_BOOL:   return mBool   == rhs.mBool   ? true : false;
  case CELL_CHAR:   return mChar   == rhs.mChar   ? true : false;
  case CELL_INT:    return mInt    == rhs.mInt    ? true : false;
  case CELL_FLOAT:  return mFloat  == rhs.mFloat  ? true : false;
  case CELL_DOUBLE: return mDouble == rhs.mDouble ? true : false;
  case CELL_STRING: return mString == rhs.mString ? true : false;
  default:          break; // To prevent warnings.
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Grid::Cell::operator!= (const Grid::Cell& rhs) const
{
  switch (mType)
  {
  case CELL_BOOL:   return mBool   != rhs.mBool   ? true : false;
  case CELL_CHAR:   return mChar   != rhs.mChar   ? true : false;
  case CELL_INT:    return mInt    != rhs.mInt    ? true : false;
  case CELL_FLOAT:  return mFloat  != rhs.mFloat  ? true : false;
  case CELL_DOUBLE: return mDouble != rhs.mDouble ? true : false;
  case CELL_STRING: return mString != rhs.mString ? true : false;
  default:          break; // To prevent warnings.
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
Grid::Cell::cellType Grid::Cell::type () const
{
  return mType;
}

////////////////////////////////////////////////////////////////////////////////
