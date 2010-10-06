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

#include "TransportSSH.h"

////////////////////////////////////////////////////////////////////////////////
TransportSSH::TransportSSH(const Uri& uri) : Transport(uri)
{
	executable = "scp";
}

////////////////////////////////////////////////////////////////////////////////
void TransportSSH::send(const std::string& source)
{
	if (uri.host == "") {
		throw std::string ("Hostname is empty");
	}

	// Is there more than one file to transfer?
	// Then path has to end with a '/'
	if (is_filelist(source) && !uri.is_directory())
    throw std::string ("'" + uri.path + "' is not a directory!");

	// cmd line is: scp [-p port] [user@]host:path
	if (uri.port != "")
	{
		arguments.push_back ("-P");
		arguments.push_back (uri.port);
	}

	arguments.push_back (source);

	if (uri.user != "")
	{
		arguments.push_back (uri.user + "@" + uri.host + ":" + uri.path);
	}
	else
	{
		arguments.push_back (uri.host + ":" + uri.path);
	}

	if (execute())
		throw std::string ("Failed to run scp!");
}

////////////////////////////////////////////////////////////////////////////////
void TransportSSH::recv(std::string target)
{
	if (uri.host == "") {
		throw std::string ("Hostname is empty");
	}

	// Is there more than one file to transfer?
	// Then target has to end with a '/'
	if (is_filelist(uri.path) && !is_directory(target))
    throw std::string ("'" + target + "' is not a directory!");

	// cmd line is: scp [-p port] [user@]host:path
	if (uri.port != "")
	{
		arguments.push_back ("-P");
		arguments.push_back (uri.port);
	}

	if (uri.user != "")
	{
		arguments.push_back (uri.user + "@" + uri.host + ":" + uri.path);
	}
	else
	{
		arguments.push_back (uri.host + ":" + uri.path);
	}

	arguments.push_back (target);

	if (execute())
		throw std::string ("Failed to run scp!");
}

////////////////////////////////////////////////////////////////////////////////
