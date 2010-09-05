////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010, Paul Beckingham, Federico Hernandez.
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

#include <sys/stat.h>
#include "Sensor.h"

////////////////////////////////////////////////////////////////////////////////
Sensor::Sensor ()
: _changed (false)
, _file ("")
, _was (0)
, _is (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Sensor::~Sensor ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Sensor::changed ()
{
  _is = getModification ();
  if (_is != _was)
    _changed = true;

  return _changed;
}

////////////////////////////////////////////////////////////////////////////////
void Sensor::reset ()
{
  _changed = false;
  _was = _is;
}

////////////////////////////////////////////////////////////////////////////////
/*
void Sensor::fileCreation (const std::string&)
{
}
*/

////////////////////////////////////////////////////////////////////////////////
void Sensor::fileModification (const std::string& file)
{
  _file = file;
  _was  = getModification ();
}

////////////////////////////////////////////////////////////////////////////////
/*
void Sensor::fileDeletion (const std::string&)
{
}
*/

////////////////////////////////////////////////////////////////////////////////
time_t Sensor::getModification ()
{
  struct stat s = {0};
  if (0 == stat (_file.c_str (), &s))
    return s.st_mtime;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
