////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_CONFIG
#define INCLUDED_CONFIG

#include <map>
#include <vector>
#include <string>
#include <FS.h>

class Config : public std::map <std::string, std::string>
{
public:
  Config ();
  Config (const Config&);
  Config& operator= (const Config&);

  void load (const std::string&, int nest = 1);
  void parse (const std::string&, int nest = 1);

  void createDefaultRC (const std::string&, const std::string&);
  void createDefaultData (const std::string&);
  void setDefaults ();
  void clear ();

  bool        has        (const std::string&);
  std::string get        (const std::string&);
  int         getInteger (const std::string&);
  double      getReal    (const std::string&);
  bool        getBoolean (const std::string&);

  void set (const std::string&, const int);
  void set (const std::string&, const double);
  void set (const std::string&, const std::string&);
  void all (std::vector <std::string>&) const;

public:
  File _original_file;

private:
  static std::string _defaults;
};

#endif

////////////////////////////////////////////////////////////////////////////////
