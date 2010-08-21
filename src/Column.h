////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_COLUMN
#define INCLUDED_COLUMN

#include <string>

class Column
{
public:
  enum just   {right = 0, left, center};
  enum sizing {minimal = 0, fixed, proportional, maximal};

  static Column* factory (const std::string&);

  Column ();
  Column (const Column&);
  Column& operator= (const Column&);
  bool operator== (const Column&) const;     // TODO Is this necessary?
  ~Column ();

  void setName (const std::string&);
  std::string render (Task*, int, int, const std::string style = "default");
  std::string type () const;

private:
  std::string _name;
  int         _minimum;
  int         _maximum;
  bool        _wrap;
  just        _just;
  sizing      _sizing;
};

class ColumnDescription : public Column
{
public:
private:
};

#endif
////////////////////////////////////////////////////////////////////////////////
