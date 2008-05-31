////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#ifndef INCLUDED_GRID
#define INCLUDED_GRID

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
class Grid
{
public:
  class Cell
  {
  public:
    Cell (const bool);
    Cell (const char);
    Cell (const int);
    Cell (const float);
    Cell (const double);
    Cell (const std::string&);

    operator bool () const;
    operator char () const;
    operator int () const;
    operator float () const;
    operator double () const;
    operator std::string () const;
    bool operator== (const Cell&) const;
    bool operator!= (const Cell&) const;

    enum cellType {CELL_BOOL, CELL_CHAR, CELL_INT, CELL_FLOAT, CELL_DOUBLE, CELL_STRING};

    cellType type () const;

  private:
    cellType mType;
    bool mBool;
    char mChar;
    int mInt;
    float mFloat;
    double mDouble;
    std::string mString;
  };

public:
  Grid ();
  ~Grid ();

  void add (const unsigned int, const unsigned int, const bool);
  void add (const unsigned int, const unsigned int, const char);
  void add (const unsigned int, const unsigned int, const int);
  void add (const unsigned int, const unsigned int, const float);
  void add (const unsigned int, const unsigned int, const double);
  void add (const unsigned int, const unsigned int, const char*);
  void add (const unsigned int, const unsigned int, const std::string&);

  unsigned int width () const;
  unsigned int height () const;

  Cell* byRow    (const unsigned int, const unsigned int) const;
  Cell* byColumn (const unsigned int, const unsigned int) const;

private:
  void expandGrid (const unsigned int, const unsigned int);
  void insertCell (const unsigned int, const unsigned int, Cell*);

private:
  std::vector < std::vector <Cell*>* > mRows;
  std::vector < std::vector <Cell*>* > mColumns;
};

#endif
////////////////////////////////////////////////////////////////////////////////

