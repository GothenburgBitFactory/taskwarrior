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
TransportSSH::TransportSSH(const std::string& uri) : Transport(uri)
{
	executable = "scp";
}

////////////////////////////////////////////////////////////////////////////////
TransportSSH::TransportSSH(
	const std::string& host,
	const std::string& path,
	const std::string& user,
	const std::string& port) : Transport (host,path,user,port)
{
	executable = "scp";
}

////////////////////////////////////////////////////////////////////////////////
void TransportSSH::send(const std::string& source)
{
	if (host == "") {
		throw std::string ("Hostname is empty");
	}

	// Is there more than one file to transfer?
	// Then path has to end with a '/'
	if ( (source.find ("*") != std::string::npos)
		|| (source.find ("?") != std::string::npos)
		|| (source.find (" ") != std::string::npos) )
	{
		std::string::size_type pos;

		pos = path.find_last_of ("/");
		if (pos != path.length()-1)
		{
			path = path.substr (0, pos+1);
		}
	}

	// cmd line is: scp [-p port] [user@]host:path
	if (port != "")
	{
		arguments.push_back ("-P");
		arguments.push_back (port);
	}

	arguments.push_back (source);

	if (user != "")
	{
		arguments.push_back (user + "@" + host + ":" + path);
	}
	else
	{
		arguments.push_back (host + ":" + path);
	}

	if (execute())
		throw std::string ("Failed to run scp!");
}

////////////////////////////////////////////////////////////////////////////////
void TransportSSH::recv(std::string target)
{
	if (host == "") {
		throw std::string ("Hostname is empty");
	}

	// Is there more than one file to transfer?
	// Then target has to end with a '/'
	if ( (path.find ("*") != std::string::npos)
		|| (path.find ("?") != std::string::npos) )
	{
		std::string::size_type pos;
		pos = target.find_last_of ("/");
		if (pos != target.length()-1)
		{
			target = target.substr( 0, pos+1);
		}
	}

	// cmd line is: scp [-p port] [user@]host:path
	if (port != "")
	{
		arguments.push_back ("-P");
		arguments.push_back (port);
	}

	if (user != "")
	{
		arguments.push_back (user + "@" + host + ":" + path);
	}
	else
	{
		arguments.push_back (host + ":" + path);
	}

	arguments.push_back (target);

	if (execute())
		throw std::string ("Failed to run scp!");
}

////////////////////////////////////////////////////////////////////////////////
