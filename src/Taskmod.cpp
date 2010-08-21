////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Johannes Schlatow.
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

#include <sstream>
#include <iostream>
#include <assert.h>
#include "Taskmod.h"

////////////////////////////////////////////////////////////////////////////////
Taskmod::Taskmod ()
{
  timestamp = 0;
  bAfterSet = false;
  bBeforeSet = false;
}

////////////////////////////////////////////////////////////////////////////////
Taskmod::Taskmod (const Taskmod& other)
{
  this->before     = other.before;
  this->after      = other.after;
  this->timestamp  = other.timestamp;
  this->bAfterSet  = other.bAfterSet;
  this->bBeforeSet = other.bBeforeSet;
}

////////////////////////////////////////////////////////////////////////////////
Taskmod::~Taskmod ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::operator< (const Taskmod &compare)
{
  return (timestamp < compare.getTimestamp ());
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::operator> (const Taskmod &compare)
{
  return (timestamp > compare.getTimestamp ());
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::operator== (const Taskmod& compare)
{
  return ( (compare.after     == this->after)
        && (compare.before    == this->before)
        && (compare.timestamp == this->timestamp) );
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::operator!= (const Taskmod& compare)
{
  return !this->operator== (compare);
}

////////////////////////////////////////////////////////////////////////////////
Taskmod& Taskmod::operator= (const Taskmod& other)
{
  if (this != &other)
  {
    this->before     = other.before;
    this->after      = other.after;
    this->timestamp  = other.timestamp;
    this->bAfterSet  = other.bAfterSet;
    this->bBeforeSet = other.bBeforeSet;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void Taskmod::reset (long timestamp)
{
  this->bAfterSet  = false;
  this->bBeforeSet = false;
  this->timestamp  = timestamp;
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::isNew ()
{
  return !bBeforeSet;
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::issetAfter ()
{
  return bAfterSet;
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::issetBefore ()
{
  return bBeforeSet;
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::isValid ()
{
  return (timestamp > 0) && (bAfterSet);
}

////////////////////////////////////////////////////////////////////////////////
std::string Taskmod::getUuid ()
{
  if (!bAfterSet)
  {
    throw std::string ("Taskmod::getUuid(): Task object not initialized.");
  }

  return after.get ("uuid");
}

////////////////////////////////////////////////////////////////////////////////
std::string Taskmod::toString ()
{
  assert (bAfterSet);

  std::stringstream stream;
  stream << "time " << timestamp << "\n";

  if (bBeforeSet)
  {
    stream << "old " << before.composeF4();
  }

  stream << "new " << after.composeF4();
  stream << "---\n";

  return stream.str ();
}

////////////////////////////////////////////////////////////////////////////////
void Taskmod::setAfter (const Task& after)
{
  this->after     = after;
  this->bAfterSet = true;
}

////////////////////////////////////////////////////////////////////////////////
void Taskmod::setBefore (const Task& before)
{
  this->before     = before;
  this->bBeforeSet = true;
}

////////////////////////////////////////////////////////////////////////////////
void Taskmod::setTimestamp (long timestamp)
{
  this->timestamp = timestamp;
}

////////////////////////////////////////////////////////////////////////////////
Task& Taskmod::getAfter ()
{
  return after;
}

////////////////////////////////////////////////////////////////////////////////
Task& Taskmod::getBefore ()
{
  return before;
}

////////////////////////////////////////////////////////////////////////////////
long Taskmod::getTimestamp () const
{
  return timestamp;
}

////////////////////////////////////////////////////////////////////////////////
std::string Taskmod::getTimeStr () const
{
  std::stringstream sstream;
  sstream << timestamp;

  return sstream.str ();
}
