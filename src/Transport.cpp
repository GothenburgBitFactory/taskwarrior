////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2013, Johannes Schlatow.
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
#include <string.h>
#include <errno.h>
#include <text.h>
#include <i18n.h>
#include <Transport.h>
#include <TransportSSH.h>
#include <TransportRSYNC.h>
#include <TransportCurl.h>
#include <TransportShell.h>

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
  else if ( uri._protocol == "sh+cp")
  {
    return new TransportShell(uri);
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
  int result = ::execute (_executable, _arguments);
  int err;
  switch (result)
  {
  case 127:
    throw format (STRING_TRANSPORT_NORUN, _executable);
  case -1:
    err = errno;
    throw format (STRING_TRANSPORT_NOFORK, _executable, ::strerror(err));
  default:
    return result;
  }
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
void Transport::expand_braces(const std::string& path,
                              const std::string& sourceortarget,
                              std::vector<std::string>& paths)
{
  // Is is_filelist appropriate here?  We only care about {}
  if (is_filelist(path))
  {
    std::string::size_type pos;
    pos = path.find("{");

    if (pos == std::string::npos)
      throw std::string (STRING_TRANSPORT_CURL_WILDCD);

    if (!is_directory(sourceortarget))
      throw format (STRING_TRANSPORT_URI_NODIR, sourceortarget);

    std::string toSplit;
    std::string suffix;
    std::string prefix = path.substr (0, pos);
    std::vector<std::string> splitted;
    toSplit = path.substr (pos+1);
    pos = toSplit.find ("}");
    suffix = toSplit.substr (pos+1);
    split (splitted, toSplit.substr(0, pos), ',');

    std::vector <std::string>::iterator file;
    for (file = splitted.begin (); file != splitted.end (); ++file) {
      std::cout << "    -- " << (prefix + *file + suffix) << "\n";
      paths.push_back (prefix + *file + suffix);
    }
  }
  else
  {
    // Not brace expandable - use the path as is.
    paths.push_back (path);
  }
}

////////////////////////////////////////////////////////////////////////////////
