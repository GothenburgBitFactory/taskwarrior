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

#include <iostream>
#include <stdlib.h>
#include <util.h>
#include <Transport.h>
#include <TransportSSH.h>
#include <TransportRSYNC.h>
#include <TransportCurl.h>

////////////////////////////////////////////////////////////////////////////////
Transport::Transport (const Uri& uri)
{
  executable = "";
  this->uri = uri;
}

////////////////////////////////////////////////////////////////////////////////
Transport::~Transport ()
{
}

////////////////////////////////////////////////////////////////////////////////
Transport* Transport::getTransport(const Uri& uri)
{
  if (uri.protocol == "ssh")
  {
    return new TransportSSH(uri);
  }
  else if (uri.protocol == "rsync")
  {
    return new TransportRSYNC(uri);
  }
  else if ( (uri.protocol == "http")
         || (uri.protocol == "https")
         || (uri.protocol == "ftp") )
  {
    return new TransportCurl(uri);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
int Transport::execute()
{
  return ::execute(executable, arguments);
}

////////////////////////////////////////////////////////////////////////////////
bool Transport::is_directory(const std::string& path)
{
  return path[path.length()-1] == '/';
}


////////////////////////////////////////////////////////////////////////////////
bool Transport::is_filelist(const std::string& path)
{
  return (path.find ("*") != std::string::npos)
      || (path.find ("?") != std::string::npos)
      || (path.find ("{") != std::string::npos);
}

////////////////////////////////////////////////////////////////////////////////

