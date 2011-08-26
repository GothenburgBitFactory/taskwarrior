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

#include <TransportCurl.h>
#include <text.h>
#include <i18n.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
TransportCurl::TransportCurl(const Uri& uri) : Transport(uri)
{
	_executable = "curl";
}

////////////////////////////////////////////////////////////////////////////////
void TransportCurl::send(const std::string& source)
{
  if (_uri._host == "")
    throw std::string (STRING_TRANSPORT_CURL_URI);

  if (_uri._user != "")
  {
    _arguments.push_back("--user");
    _arguments.push_back(_uri._user);
  }

  if (is_filelist(source))
  {
    std::string::size_type pos;
    pos = source.find("{");

    if (pos == std::string::npos)
      throw std::string (STRING_TRANSPORT_CURL_WILDCD);

    if (!_uri.is_directory())
      throw format (STRING_TRANSPORT_URI_NODIR, _uri._path);

    _arguments.push_back ("-T");
    _arguments.push_back ("\"" + source + "\"");
  }
  else
  {
    _arguments.push_back ("-T");
    _arguments.push_back (source);
  }

  // cmd line is: curl -T source protocol://host:port/path
  if (_uri._port != "")
  {
    _arguments.push_back (_uri._protocol + "://" + _uri._host + ":" + _uri._port + "/" + _uri._path);
  }
  else
  {
    _arguments.push_back (_uri._protocol + "://" + _uri._host + "/" + _uri._path);
  }

  int result = execute();
  if (result)
  {
    if (result == 127) // command not found
      throw std::string (STRING_TRANSPORT_CURL_NORUN);
    else
      throw std::string (STRING_TRANSPORT_CURL_FAIL);
  }
}

////////////////////////////////////////////////////////////////////////////////
void TransportCurl::recv(std::string target)
{
  if (_uri._host == "")
    throw std::string (STRING_TRANSPORT_CURL_URI);

  if (_uri._user != "")
  {
    _arguments.push_back("--user");
    _arguments.push_back(_uri._user);
  }


  if (is_filelist(_uri._path))
  {
    std::string::size_type pos;
    pos = _uri._path.find("{");

    if (pos == std::string::npos)
      throw std::string (STRING_TRANSPORT_CURL_WILDCD);

    if (!is_directory(target))
      throw format (STRING_TRANSPORT_URI_NODIR, target);

    std::string toSplit;
    std::string suffix;
    std::string prefix = target;
    std::vector<std::string> splitted;
    toSplit = _uri._path.substr (pos+1);
    pos = toSplit.find ("}");
    suffix = toSplit.substr (pos+1);
    split (splitted, toSplit.substr(0, pos), ',');

    target = "";

    std::vector <std::string>::iterator file;
    for (file = splitted.begin (); file != splitted.end (); ++file)
      target += " -o " + prefix + *file + suffix;
  }
  else
  {
    target = "-o " + target;
  }

  // cmd line is: curl protocol://host:port/path/to/source/file -o path/to/target/file
  if (_uri._port != "")
  {
    _arguments.push_back (_uri._protocol + "://" + _uri._host + ":" + _uri._port + "/" + _uri._path);
  }
  else
  {
    _arguments.push_back (_uri._protocol + "://" + _uri._host + "/" + _uri._path);
  }

  _arguments.push_back (target);

  int result = execute();
  if (result)
  {
    if (result == 127) // command not found
      throw std::string (STRING_TRANSPORT_CURL_NORUN);
    else
      throw std::string (STRING_TRANSPORT_CURL_FAIL);
  }
}

////////////////////////////////////////////////////////////////////////////////
// vim: ts=2 sw=2 et
