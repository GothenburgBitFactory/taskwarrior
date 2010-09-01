////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Johannes Schlatow.
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

#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Transport.h"
#include "TransportSSH.h"

////////////////////////////////////////////////////////////////////////////////
Transport::Transport (const std::string& host, const std::string& path, const std::string& user="", const std::string& port="")
{
  executable = "";
	this->host = host;
	this->path = path;
	this->user = user;
	this->port = port;
}

////////////////////////////////////////////////////////////////////////////////
Transport::Transport (const std::string& uri)
{
  executable = "";
	
	parseUri(uri);
}

////////////////////////////////////////////////////////////////////////////////
Transport::~Transport ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Transport::parseUri(std::string uri)
{
  std::string::size_type pos;	
	std::string uripart;
	
	user = "";
	port = "";
	
	// skip ^.*://
  if ((pos = uri.find ("://")) != std::string::npos)
	{
		uri = uri.substr (pos+3);
	}
	
	// get host part
	if ((pos = uri.find ("/")) != std::string::npos)
	{
		host = uri.substr (0, pos);
		path = uri.substr (pos+1);
	}
	else
	{
		throw std::string ("Could not parse \""+uri+"\"");
	}
	
	// parse host
	if ((pos = host.find ("@")) != std::string::npos)
	{
		user = host.substr (0, pos);
		host = host.substr (pos+1);
	}
	
	if ((pos = host.find (":")) != std::string::npos)
	{
		port = host.substr (pos+1);
		host = host.substr (0,pos);
	}
}

////////////////////////////////////////////////////////////////////////////////
Transport* Transport::getTransport(const std::string& uri)
{
	if (uri.find("ssh://") == 0) {
			return new TransportSSH(uri);
	}
	
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
int Transport::execute()
{
	if (executable == "") 
		return -1;
		
	pid_t child_pid = fork();
	
	if (child_pid == 0)
	{
		// this is done by the child process		
		char shell[] = "sh";
		char opt[]   = "-c";
	
		std::string cmdline = executable;
		
		std::vector <std::string>::iterator it;
		for (it = arguments.begin(); it != arguments.end(); ++it)
		{
			std::string tmp = *it;
			cmdline += " " + tmp;			
		}
		
		char** argv = new char*[4];
		argv[0] = shell;                  // sh
		argv[1] = opt;                    // -c
		argv[2] = (char*)cmdline.c_str();	// e.g. scp undo.data user@host:.task/
		argv[3] = NULL;                   // required by execv

		int ret = execvp("sh", argv);	
		delete[] argv;
		
		exit(ret);
	}
	else
	{
		// this is done by the parent process
		int child_status;
		
		pid_t pid = waitpid(child_pid, &child_status, 0);
		
		if (pid == -1)
			return -1;
		else
			return child_status;
	}
}