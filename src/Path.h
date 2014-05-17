////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_PATH
#define INCLUDED_PATH

#include <vector>
#include <string>

class Path
{
public:
  Path ();
  Path (const Path&);
  Path (const std::string&);
  virtual ~Path ();

  Path& operator= (const Path&);
  bool operator== (const Path&);
  Path& operator+= (const std::string&);
  operator std::string () const;

  std::string name () const;
  std::string parent () const;
  std::string extension () const;
  bool exists () const;
  bool is_directory () const;
  bool is_absolute () const;
  bool is_link () const;
  bool readable () const;
  bool writable () const;
  bool executable () const;
  bool rename (const std::string&);

  // Statics
  static std::string expand (const std::string&);
  static std::vector<std::string> glob (const std::string&);

public:
  std::string _original;
  std::string _data;
};

std::ostream& operator<< (std::ostream&, const Path&);

#endif
////////////////////////////////////////////////////////////////////////////////
