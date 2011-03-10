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

#include "Context.h"
#include "Uri.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Uri::Uri ()
{
  parsed = false;
}

////////////////////////////////////////////////////////////////////////////////
Uri::Uri (const Uri& other)
{
  if (this != &other)
  {
    data = other.data;
    host = other.host;
    path = other.path;
    user = other.user;
    port = other.port;
    protocol = other.protocol;
    parsed = other.parsed;
  }
}

////////////////////////////////////////////////////////////////////////////////
Uri::Uri (const std::string& in, const std::string& configPrefix)
{
  data = in;
  parsed = false;
  if (configPrefix != "")
    expand(configPrefix);
}

////////////////////////////////////////////////////////////////////////////////
Uri::~Uri ()
{
}

////////////////////////////////////////////////////////////////////////////////
Uri& Uri::operator= (const Uri& other)
{
  if (this != &other)
  {
    this->data = other.data;
    this->host = other.host;
    this->path = other.path;
    this->user = other.user;
    this->port = other.port;
    this->protocol = other.protocol;
	 this->parsed = other.parsed;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Uri::operator std::string () const
{
  return data;
}

////////////////////////////////////////////////////////////////////////////////
std::string Uri::name () const
{
  if (path.length ())
  {
    std::string::size_type slash = path.rfind ('/');
    if (slash != std::string::npos)
      return path.substr (slash + 1, std::string::npos);
  }

 return path;
}

////////////////////////////////////////////////////////////////////////////////
std::string Uri::parent () const
{
  if (path.length ())
  {
    std::string::size_type slash = path.rfind ('/');
    if (slash != std::string::npos)
      return path.substr (0, slash+1);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string Uri::extension () const
{
  if (path.length ())
  {
    std::string::size_type dot = path.rfind ('.');
    if (dot != std::string::npos)
      return path.substr (dot + 1, std::string::npos);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
bool Uri::is_directory () const
{
  if (is_local ()) {
    return Path (this->data).is_directory ();
  } else
    return (path == ".")
        || (path == "")
        || (path[path.length()-1] == '/');
}

////////////////////////////////////////////////////////////////////////////////
bool Uri::is_local () const
{
  if (parsed)
    return (protocol == "");
  else
    return ( (data.find("://") == std::string::npos)
          && (data.find(":")   == std::string::npos) );
}

////////////////////////////////////////////////////////////////////////////////
bool Uri::append (const std::string& path)
{
  if (is_directory ())
  {
    this->path += path;
    return true;
  }
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Uri::expand (const std::string& configPrefix )
{
  std::string tmp;
  if (data.length ())
  {
    // try to replace argument with uri from config
    tmp = context.config.get (configPrefix + "." + data + ".uri");
  }
  else
  {
    // get default target from config
    tmp = context.config.get (configPrefix + ".default.uri");
  }

  if (tmp != "")
  {
    data = tmp;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Uri::parse ()
{
  if (parsed)
    return;

  if (is_local ())
  {
    path = data;
    parsed = true;
    return;
  }

  std::string::size_type pos;
  std::string data = this->data;
  std::string pathDelimiter = "/";

  user = "";
  port = "";

	// skip ^.*://
  if ((pos = data.find ("://")) != std::string::npos)
  {
    protocol = data.substr(0, pos);
    data = data.substr (pos+3);
    // standard syntax: protocol://[user@]host.xz[:port]/path/to/undo.data
    pathDelimiter = "/";
  }
  else
  {
    protocol = "ssh";
    // scp-like syntax: [user@]host.xz:path/to/undo.data
    pathDelimiter = ":";
  }

  // user delimited by single quotes?
  if ( data[0] == '\''
   && (pos = data.find("'", 1)) != std::string::npos )
  {
    if (data[pos+1] == '@')
    {
      // end of user name
      user = data.substr (1, pos-1);
      data = data.substr (pos+2);
    }
    else
    {
      throw std::string ("Could not parse uri '") + data + "', wrong usage of single quotes.";
    }
  }
  else
  {
    // find user name
    if ((pos = data.find ("@")) != std::string::npos)
    {
      user = data.substr (0, pos);
      data = data.substr (pos+1);
    }
  }

  // get host, port and path
  if ((pos = data.find (pathDelimiter)) != std::string::npos)
  {
    host = data.substr (0, pos);
    path = data.substr (pos+1);
  }
  else
  {
    throw std::string ("The uri '") + data + "' is not in the expected format.";
  }

  // port specified?
  // remark: this find() will never be != npos for scp-like syntax
  // because we found pathDelimiter, which is ":", before
  if ((pos = host.find (":")) != std::string::npos)
  {
    port = host.substr (pos+1);
    host = host.substr (0,pos);
  }

  parsed = true;
}

////////////////////////////////////////////////////////////////////////////////
// vim: et ts=2 sw=2
