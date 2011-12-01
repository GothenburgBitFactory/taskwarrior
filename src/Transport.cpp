////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Johannes Schlatow.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////


#define L10N                                           // Localization complete.

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
  _executable = "";
  this->_uri = uri;
}

////////////////////////////////////////////////////////////////////////////////
Transport::~Transport ()
{
}

////////////////////////////////////////////////////////////////////////////////
Transport* Transport::getTransport(const Uri& uri)
{
  if (uri._protocol == "ssh")
  {
    return new TransportSSH(uri);
  }
  else if (uri._protocol == "rsync")
  {
    return new TransportRSYNC(uri);
  }
  else if ( (uri._protocol == "http")
         || (uri._protocol == "https")
         || (uri._protocol == "ftp") )
  {
    return new TransportCurl(uri);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
int Transport::execute()
{
  // quote arguments
  std::vector<std::string>::iterator it = _arguments.begin ();
  for (; it != _arguments.end (); it++)
  {
    // quote until the first appearance of '{'
    size_t pos = it->find('{');
    if (pos != 0)
    {
      // '{' is not the first character
      it->insert(0, "\"");
      if (pos != std::string::npos)
        it->insert(pos+1, "\"");
      else
        it->append("\"");
    }
  }
  return ::execute(_executable, _arguments);
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

