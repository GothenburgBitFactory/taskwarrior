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
  std::vector<std::string> sourcelist;
  std::vector<std::string>::const_iterator source_iter;

  if (_uri._host == "")
    throw std::string (STRING_TRANSPORT_CURL_URI);

  if (_uri._user != "")
  {
    _arguments.push_back("--user");
    _arguments.push_back(_uri._user);
  }

  if (is_filelist (source)) {
    expand_braces (source, _uri._data, sourcelist);
    // Is there more than one source?
    // Then path has to end with a '/'
    if (sourcelist.size () > 1 && !_uri.is_directory ())
      throw format (STRING_TRANSPORT_URI_NODIR, _uri);

    for (source_iter = sourcelist.begin (); source_iter != sourcelist.end (); ++source_iter) {
      _arguments.push_back ("-T");
      _arguments.push_back ("\"" + escape (*source_iter, ' ') + "\"");
    }
  }
  else
  {
    _arguments.push_back ("-T");
    _arguments.push_back ("\"" + escape (source, ' ') + "\"");
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

  if (execute ())
    throw std::string (STRING_TRANSPORT_CURL_FAIL);
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

  std::vector<std::string> targetargs;

  if (is_filelist(_uri._path))
  {

    std::vector<std::string> paths;
    expand_braces (_uri._path, target, paths);

    std::vector <std::string>::iterator file;
    for (file = paths.begin (); file != paths.end (); ++file) {
      targetargs.push_back ("-o");
      targetargs.push_back (*file);
    }
  }
  else
  {
    targetargs.push_back ("-o");
    targetargs.push_back (target);
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

  _arguments.insert (_arguments.end (), targetargs.begin (), targetargs.end ());

  if (execute ())
    throw std::string (STRING_TRANSPORT_CURL_FAIL);
}

////////////////////////////////////////////////////////////////////////////////
// vim: ts=2 sw=2 et
