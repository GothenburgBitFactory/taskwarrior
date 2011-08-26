////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Johannes Schlatow.
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

#define L10N                                           // Localization complete.

#include <sstream>
#include <iostream>
#include <i18n.h>
#include <assert.h>
#include <Taskmod.h>

////////////////////////////////////////////////////////////////////////////////
Taskmod::Taskmod ()
{
  _timestamp = 0;
  _bAfterSet = false;
  _bBeforeSet = false;
}

////////////////////////////////////////////////////////////////////////////////
Taskmod::Taskmod (const Taskmod& other)
{
  this->_before     = other._before;
  this->_after      = other._after;
  this->_timestamp  = other._timestamp;
  this->_bAfterSet  = other._bAfterSet;
  this->_bBeforeSet = other._bBeforeSet;
}

////////////////////////////////////////////////////////////////////////////////
Taskmod::~Taskmod ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::operator< (const Taskmod &compare)
{
  return (_timestamp < compare.getTimestamp ());
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::operator> (const Taskmod &compare)
{
  return (_timestamp > compare.getTimestamp ());
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::operator== (const Taskmod& compare)
{
  return ( (compare._after     == this->_after)
        && (compare._before    == this->_before)
        && (compare._timestamp == this->_timestamp) );
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
    this->_before     = other._before;
    this->_after      = other._after;
    this->_timestamp  = other._timestamp;
    this->_bAfterSet  = other._bAfterSet;
    this->_bBeforeSet = other._bBeforeSet;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void Taskmod::reset (long timestamp)
{
  this->_bAfterSet  = false;
  this->_bBeforeSet = false;
  this->_timestamp  = timestamp;
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::isNew ()
{
  return !_bBeforeSet;
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::issetAfter ()
{
  return _bAfterSet;
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::issetBefore ()
{
  return _bBeforeSet;
}

////////////////////////////////////////////////////////////////////////////////
bool Taskmod::isValid ()
{
  return (_timestamp > 0) && (_bAfterSet);
}

////////////////////////////////////////////////////////////////////////////////
std::string Taskmod::getUuid ()
{
  if (!_bAfterSet)
    throw std::string (STRING_TASKMOD_BAD_INIT);

  return _after.get ("uuid");
}

////////////////////////////////////////////////////////////////////////////////
std::string Taskmod::toString ()
{
  assert (_bAfterSet);

  std::stringstream stream;
  stream << STRING_TASKMOD_TIME << _timestamp << "\n";

  if (_bBeforeSet)
  {
    stream << STRING_TASKMOD_OLD << _before.composeF4();
  }

  stream << STRING_TASKMOD_NEW << _after.composeF4();
  stream << "---\n";

  return stream.str ();
}

////////////////////////////////////////////////////////////////////////////////
void Taskmod::setAfter (const Task& after)
{
  this->_after     = after;
  this->_bAfterSet = true;
}

////////////////////////////////////////////////////////////////////////////////
void Taskmod::setBefore (const Task& before)
{
  this->_before     = before;
  this->_bBeforeSet = true;
}

////////////////////////////////////////////////////////////////////////////////
void Taskmod::setTimestamp (long timestamp)
{
  this->_timestamp = timestamp;
}

////////////////////////////////////////////////////////////////////////////////
Task& Taskmod::getAfter ()
{
  return _after;
}

////////////////////////////////////////////////////////////////////////////////
Task& Taskmod::getBefore ()
{
  return _before;
}

////////////////////////////////////////////////////////////////////////////////
long Taskmod::getTimestamp () const
{
  return _timestamp;
}

////////////////////////////////////////////////////////////////////////////////
std::string Taskmod::getTimeStr () const
{
  std::stringstream sstream;
  sstream << _timestamp;

  return sstream.str ();
}

////////////////////////////////////////////////////////////////////////////////
