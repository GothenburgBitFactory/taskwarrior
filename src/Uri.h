////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2012, Johannes Schlatow.
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

#ifndef INCLUDED_URI
#define INCLUDED_URI
#define L10N                                           // Localization complete.

#include <vector>
#include <string>

// supports the following syntaxes:
// protocol://[user@]host.tld[:port]/path
// [user@]host:path
// path/to/local/file.ext
// alias (e.g. merge.alias.uri)
class Uri
{
public:
  Uri ();
  Uri (const Uri&);
  Uri (const std::string&, const std::string& configPrefix="");
  virtual ~Uri ();

  Uri& operator= (const Uri&);
  operator std::string () const;

  std::string name () const;
  std::string parent () const;
  std::string extension () const;
  std::string ToString();
  bool is_directory () const;
  bool is_local () const;
  bool append (const std::string&);
  bool expand (const std::string&);
  void parse ();

public:
  std::string _data;
  std::string _path;
  std::string _host;
  std::string _port;
  std::string _user;
  std::string _protocol;
  bool _parsed;
};

#endif
////////////////////////////////////////////////////////////////////////////////
