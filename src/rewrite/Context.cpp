////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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

#include "Context.h"

////////////////////////////////////////////////////////////////////////////////
Context::Context ()
{
}

////////////////////////////////////////////////////////////////////////////////
Context::Context (const Context& other)
{
  throw std::string ("unimplemented Context::Context");
//  config   = other.config;
  filter   = other.filter;
  keymap   = other.keymap;
  sequence = other.sequence;
  task     = other.task;
  tdb      = other.tdb;
}

////////////////////////////////////////////////////////////////////////////////
Context& Context::operator= (const Context& other)
{
  throw std::string ("unimplemented Context::operator=");
  if (this != &other)
  {
//    config   = other.config;
    filter   = other.filter;
    keymap   = other.keymap;
    sequence = other.sequence;
    task     = other.task;
    tdb      = other.tdb;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Context::~Context ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Context::initialize ()
{
  throw std::string ("unimplemented Context::initialize");
  // TODO Load config.
  // TODO Load pending.data.
  // TODO Load completed.data.
  // TODO Load deleted.data.
}

////////////////////////////////////////////////////////////////////////////////
int Context::commandLine (int argc, char** argv)
{
  throw std::string ("unimplemented Context::commandLine");
  // TODO Support rc: override.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  throw std::string ("unimplemented Context::run");
  // TODO Dispatch to command handlers.
  // TODO Auto shadow update.
  // TODO Auto gc.

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

