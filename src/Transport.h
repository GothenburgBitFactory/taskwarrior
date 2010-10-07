////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010, Johannes Schlatow.
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
#ifndef INCLUDED_TRANSPORT
#define INCLUDED_TRANSPORT

#include <string>
#include <vector>
#include "Uri.h"

class Transport {
public:
  Transport (const Uri&);
  ~Transport ();

  static Transport* getTransport(const Uri&);

  virtual void send (const std::string&) = 0;
  virtual void recv (std::string) = 0;

  static bool is_directory(const std::string&);
  static bool is_filelist(const std::string&);

protected:
  std::string executable;
  std::vector<std::string> arguments;

  Uri uri;

  int execute();
};

#endif

