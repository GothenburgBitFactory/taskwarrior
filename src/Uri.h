////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Johannes Schlatow.
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
#ifndef INCLUDED_URI
#define INCLUDED_URI

#include <vector>
#include <string>

// supports the following syntaxes:
// protocol://[user@]host.tld[:port]/path
// [user@]host:path
// path/to/local/file.ext
// alias (e.g. merge.alias.uri)
class Uri
{
public:
  Uri ();
  Uri (const Uri&);
  Uri (const std::string&, const std::string& configPrefix="");
  virtual ~Uri ();

  Uri& operator= (const Uri&);
  operator std::string () const;

  std::string name () const;
  std::string parent () const;
  std::string extension () const;
  bool is_directory () const;
  bool is_local () const;
  bool append (const std::string&);
  bool expand (const std::string&);
  void parse ();

public:
  std::string data;
  std::string path;
  std::string host;
  std::string port;
  std::string user;
  std::string protocol;
  bool parsed;
};

#endif
////////////////////////////////////////////////////////////////////////////////
