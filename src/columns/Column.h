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
#define L10N                                           // Localization complete.

#include <vector>
#include <string>
#include <Color.h>
#include <Task.h>

class Column
{
public:
  static Column* factory (const std::string&, const std::string&);
  static void factory (std::map <std::string, Column*>&);

  Column ();
  Column (const Column&);
  Column& operator= (const Column&);
  bool operator== (const Column&) const;     // TODO Is this necessary?
  ~Column ();

  std::string style () const                  { return _style;  }
  std::string label () const                  { return _label;  }
  std::string type () const                   { return _type;   }
  std::vector <std::string> styles () const   { return _styles; }
  std::vector <std::string> examples () const { return _examples; }

  virtual void setStyle  (const std::string& value) { _style = value;  }
  virtual void setLabel  (const std::string& value) { _label = value;  }
  virtual void setReport (const std::string& value) { _report = value; }

  virtual bool validate (std::string&);
  virtual void measure (const std::string&, int&, int&);
  virtual void measure (Task&, int&, int&);
  virtual void renderHeader (std::vector <std::string>&, int, Color&);
  virtual void render (std::vector <std::string>&, const std::string&, int, Color&);
  virtual void render (std::vector <std::string>&, Task&, int, Color&);

protected:
  std::string _name;
  std::string _type;
  std::string _style;
  std::string _label;
  std::string _report;
  std::vector <std::string> _styles;
  std::vector <std::string> _examples;
};

#endif
////////////////////////////////////////////////////////////////////////////////
