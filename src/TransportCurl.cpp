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

////////////////////////////////////////////////////////////////////////////////
TransportCurl::TransportCurl(const std::string& uri) : Transport(uri)
{
	executable = "curl";
	
	if (protocol == "")
		protocol = "http";
}

////////////////////////////////////////////////////////////////////////////////
TransportCurl::TransportCurl(
	const std::string& host,
	const std::string& path,
	const std::string& user,
	const std::string& port) : Transport (host,path,user,port)
{
	executable = "curl";
	
	if (protocol == "")
		protocol = "http";
}

////////////////////////////////////////////////////////////////////////////////
void TransportCurl::send(const std::string& source)
{
	if (host == "") {
		throw std::string ("Hostname is empty");
	}

	// Wildcards arent supported
	if ( (source.find ("*") != std::string::npos)
		|| (source.find ("?") != std::string::npos) )
	{
		throw std::string ("Failed to use curl with wildcards!");
	}

	// cmd line is: curl -T source protocol://host:port/path
	arguments.push_back ("-T");
	arguments.push_back (source);

	if (port != "")
	{
		arguments.push_back (protocol + "://" + host + ":" + port + "/" + path);
	}
	else
	{
		arguments.push_back (protocol + "://" + host + "/" + path);
	}

	if (execute())
		throw std::string ("Failed to run curl!");
}

////////////////////////////////////////////////////////////////////////////////
void TransportCurl::recv(std::string target)
{
	if (host == "") {
		throw std::string ("Hostname is empty");
	}

	// Wildcards arent supported
	if ( (path.find ("*") != std::string::npos)
		|| (path.find ("?") != std::string::npos) )
	{
		throw std::string ("Failed to use curl with wildcards!");
	}
	
	// cmd line is: curl protocol://host:port/path/to/source/file -o path/to/target/file
	if (port != "")
	{
		arguments.push_back (protocol + "://" + host + ":" + port + "/" + path);
	}
	else
	{
		arguments.push_back (protocol + "://" + host + "/" + path);
	}
	
	arguments.push_back ("-o");
	arguments.push_back (target);

	if (execute())
		throw std::string ("Failed to run curl!");
}

////////////////////////////////////////////////////////////////////////////////
