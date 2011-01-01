////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_SENSOR
#define INCLUDED_SENSOR

#include <string>

class Sensor
{
public:
  Sensor ();
  ~Sensor ();
  Sensor (const Sensor&);
  Sensor& operator= (const Sensor&);

  bool changed ();
  void reset ();

//  void fileCreation     (const std::string&);
  void fileModification (const std::string&);
//  void fileDeletion     (const std::string&);

private:
  time_t getModification ();

private:
  bool        _changed;
  std::string _file;
  time_t      _was;
  time_t      _is;
};

#endif

////////////////////////////////////////////////////////////////////////////////
