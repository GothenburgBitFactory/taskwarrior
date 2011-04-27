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
#ifndef INCLUDED_COLUMN
#define INCLUDED_COLUMN

#include <vector>
#include <string>
#include <Task.h>

class Column
{
public:
  static Column* factory (const std::string&);

  Column ();
  Column (const Column&);
  Column& operator= (const Column&);
  bool operator== (const Column&) const;     // TODO Is this necessary?
  ~Column ();

  std::string getStyle ()                  { return _style;  }
  std::string getLabel ()                  { return _label;  }
  void setStyle (const std::string& value) { _style = value; }
  void setLabel (const std::string& value) { _label = value; }
  std::string type () const                { return _type;   }

  virtual void measure (Task&, int&, int&) = 0;
  virtual void renderHeader (std::vector <std::string>&, int);
  virtual void render (std::vector <std::string>&, Task&, int) = 0;

protected:
  std::string _type;
  std::string _style;
  std::string _label;
};

#endif
////////////////////////////////////////////////////////////////////////////////
