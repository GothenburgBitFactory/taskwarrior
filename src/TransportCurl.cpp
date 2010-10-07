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

#include "TransportCurl.h"
#include "text.h"
#include "util.h"

////////////////////////////////////////////////////////////////////////////////
TransportCurl::TransportCurl(const Uri& uri) : Transport(uri)
{
	executable = "curl";
}

////////////////////////////////////////////////////////////////////////////////
void TransportCurl::send(const std::string& source)
{
	if (uri.host == "")
		throw std::string ("when using the 'curl' protocol, the uri must contain a hostname.");

	if (is_filelist(source))
  {
    std::string::size_type pos;
    pos = source.find("{");

    if (pos == std::string::npos)
      throw std::string ("When using the 'curl' protocol, wildcards are not supported.");

    if (!uri.is_directory())
      throw std::string ("The uri '"); + uri.path + "' does not appear to be a directory.";

    std::string toSplit;
    std::string suffix;
    std::string prefix;
    std::vector<std::string> splitted;

    prefix  = source.substr (0, pos);
    toSplit = source.substr (pos+1);

    pos     = toSplit.find ("}");
    suffix  = toSplit.substr (pos+1);
    split (splitted, toSplit.substr(0, pos), ',');

    foreach (file, splitted)
    {
      arguments.push_back ("-T");
      arguments.push_back (prefix + *file + suffix);
    }
  }
  else
  {
    arguments.push_back ("-T");
    arguments.push_back (source);
  }

	// cmd line is: curl -T source protocol://host:port/path
	if (uri.port != "")
	{
		arguments.push_back (uri.protocol + "://" + uri.host + ":" + uri.port + "/" + uri.path);
	}
	else
	{
		arguments.push_back (uri.protocol + "://" + uri.host + "/" + uri.path);
	}

	if (execute())
    throw std::string ("Could not run curl.  Is it installed, and available in $PATH?");
}

////////////////////////////////////////////////////////////////////////////////
void TransportCurl::recv(std::string target)
{
	if (uri.host == "")
		throw std::string ("when using the 'curl' protocol, the uri must contain a hostname.");

  if (is_filelist(uri.path))
  {
    std::string::size_type pos;
    pos = uri.path.find("{");

    if (pos == std::string::npos)
      throw std::string ("When using the 'curl' protocol, wildcards are not supported.");

    if (!is_directory(target))
      throw std::string ("The uri '"); + target + "' does not appear to be a directory.";

    std::string toSplit;
    std::string suffix;
    std::string prefix = target;
    std::vector<std::string> splitted;
    toSplit = uri.path.substr (pos+1);
    pos = toSplit.find ("}");
    suffix = toSplit.substr (pos+1);
    split (splitted, toSplit.substr(0, pos), ',');

    target = "";
    foreach (file, splitted)
    {
      target += " -o " + prefix + *file + suffix;
    }
  }
  else
  {
    target = "-o " + target;
  }

	// cmd line is: curl protocol://host:port/path/to/source/file -o path/to/target/file
	if (uri.port != "")
	{
		arguments.push_back (uri.protocol + "://" + uri.host + ":" + uri.port + "/" + uri.path);
	}
	else
	{
		arguments.push_back (uri.protocol + "://" + uri.host + "/" + uri.path);
	}

	arguments.push_back (target);

	if (execute())
    throw std::string ("Could not run curl.  Is it installed, and available in $PATH?");
}

////////////////////////////////////////////////////////////////////////////////
