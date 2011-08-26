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

#define L10N                                           // Localization complete.

#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <Uri.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Uri::Uri ()
{
  _parsed = false;
}

////////////////////////////////////////////////////////////////////////////////
Uri::Uri (const Uri& other)
{
  if (this != &other)
  {
    _data = other._data;
    _host = other._host;
    _path = other._path;
    _user = other._user;
    _port = other._port;
    _protocol = other._protocol;
    _parsed = other._parsed;
  }
}

////////////////////////////////////////////////////////////////////////////////
Uri::Uri (const std::string& in, const std::string& configPrefix)
{
  _data = in;
  _parsed = false;
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
    this->_data = other._data;
    this->_host = other._host;
    this->_path = other._path;
    this->_user = other._user;
    this->_port = other._port;
    this->_protocol = other._protocol;
    this->_parsed = other._parsed;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Uri::operator std::string () const
{
  return _data;
}

////////////////////////////////////////////////////////////////////////////////
std::string Uri::name () const
{
  if (_path.length ())
  {
    std::string::size_type slash = _path.rfind ('/');
    if (slash != std::string::npos)
      return _path.substr (slash + 1, std::string::npos);
  }

 return _path;
}

////////////////////////////////////////////////////////////////////////////////
std::string Uri::parent () const
{
  if (_path.length ())
  {
    std::string::size_type slash = _path.rfind ('/');
    if (slash != std::string::npos)
      return _path.substr (0, slash+1);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string Uri::extension () const
{
  if (_path.length ())
  {
    std::string::size_type dot = _path.rfind ('.');
    if (dot != std::string::npos)
      return _path.substr (dot + 1, std::string::npos);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
bool Uri::is_directory () const
{
  if (is_local ()) {
    return Path (this->_data).is_directory ();
  } else
    return (_path == ".")
        || (_path == "")
        || (_path[_path.length()-1] == '/');
}

////////////////////////////////////////////////////////////////////////////////
bool Uri::is_local () const
{
  if (_parsed)
    return (_protocol == "");
  else
    return ( (_data.find("://") == std::string::npos)
          && (_data.find(":")   == std::string::npos) );
}

////////////////////////////////////////////////////////////////////////////////
bool Uri::append (const std::string& path)
{
  if (is_directory ())
  {
    this->_path += path;
    return true;
  }
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Uri::expand (const std::string& configPrefix )
{
  std::string tmp;
  if (_data.length ())
  {
    // try to replace argument with uri from config
    tmp = context.config.get (configPrefix + "." + _data + ".uri");
  }
  else
  {
    // get default target from config
    tmp = context.config.get (configPrefix + ".default.uri");
  }

  if (tmp != "")
  {
    _data = tmp;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Uri::parse ()
{
  if (_parsed)
    return;

  if (is_local ())
  {
    _path = _data;
    _parsed = true;
    return;
  }

  std::string::size_type pos;
  std::string _data = this->_data;
  std::string pathDelimiter = "/";

  _user = "";
  _port = "";

	// skip ^.*://
  if ((pos = _data.find ("://")) != std::string::npos)
  {
    _protocol = _data.substr(0, pos);
    _data = _data.substr (pos+3);
    // standard syntax: protocol://[user@]host.xz[:port]/path/to/undo.data
    pathDelimiter = "/";
  }
  else
  {
    _protocol = "ssh";
    // scp-like syntax: [user@]host.xz:path/to/undo.data
    pathDelimiter = ":";
  }

  // user delimited by single quotes?
  if ( _data[0] == '\''
   && (pos = _data.find("'", 1)) != std::string::npos )
  {
    if (_data[pos+1] == '@')
    {
      // end of user name
      _user = _data.substr (1, pos-1);
      _data = _data.substr (pos+2);
    }
    else
    {
      throw std::string (format (STRING_URI_QUOTES, _data));
    }
  }
  else
  {
    // find user name
    if ((pos = _data.find ("@")) != std::string::npos)
    {
      _user = _data.substr (0, pos);
      _data = _data.substr (pos+1);
    }
  }

  // get host, port and path
  if ((pos = _data.find (pathDelimiter)) != std::string::npos)
  {
    _host = _data.substr (0, pos);
    _path = _data.substr (pos+1);
  }
  else
  {
    throw std::string (format (STRING_URI_BAD_FORMAT, _data));
  }

  // path is absolute for ssh:// syntax
  if ( (_protocol == "ssh") && (pathDelimiter == "/") )
  {
    _path = "/" + _path;
  }

  // port specified?
  // remark: this find() will never be != npos for scp-like syntax
  // because we found pathDelimiter, which is ":", before
  if ((pos = _host.find (":")) != std::string::npos)
  {
    _port = _host.substr (pos+1);
    _host = _host.substr (0,pos);
  }

  _parsed = true;
}

////////////////////////////////////////////////////////////////////////////////
// vim: et ts=2 sw=2
