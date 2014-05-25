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

#ifndef INCLUDED_A3
#define INCLUDED_A3

#include <vector>
#include <string>
#include <Arg.h>
#include <Nibbler.h>
#include <File.h>

class A3 : public std::vector <Arg>
{
public:
  A3 ();
  A3 (const A3&);
  A3& operator= (const A3&);
  ~A3 ();

  void capture (int, const char**);
  void capture (const std::string&);
  void capture_first (const std::string&);

  void categorize ();
  static bool is_command (const std::vector <std::string>&, std::string&);
  static const std::vector <std::string> operator_list ();

  void append_stdin ();
  void resolve_aliases ();
  void apply_overrides ();
  void inject_defaults ();

  const std::string combine () const;
  const std::vector <std::string> list () const;
  bool find_command (std::string&) const;

  const A3 tokenize (const A3&) const;

  static bool is_attr (Nibbler&, Arg&);
  static bool is_attmod (Nibbler&, Arg&);
  static bool is_attribute (const std::string&, std::string&);
  static bool is_modifier (const std::string&, std::string&);
  static bool is_dom (Nibbler&, Arg&);
  static bool is_date (Nibbler&, std::string&);
  static bool is_duration (Nibbler&, std::string&);
  static bool is_pattern (Nibbler&, std::string&);
  static bool is_subst (Nibbler&, std::string&);
  static bool is_id (Nibbler&, std::string&);
  static bool is_uuid (Nibbler&, std::string&);
  static bool is_tag (Nibbler&, std::string&);
  static bool is_number (Nibbler&, std::string&);
  static bool is_integer (Nibbler&, int&);
  static bool is_operator (std::vector <std::string>&, Nibbler&, std::string&);

  static bool extract_pattern (const std::string&, std::string&);
  static bool extract_tag (const std::string&, char&, std::string&);
  static bool extract_attr (const std::string&, std::string&, std::string&);
  static bool extract_attmod (const std::string&, std::string&, std::string&, std::string&, std::string&);
  static bool extract_subst (const std::string&, std::string&, std::string&, bool&);
  static bool extract_id (const std::string&, std::vector <int>&);
  static bool extract_uuid (const std::string&, std::vector <std::string>&);

  static bool which_operator (const std::string&, char&, int&, char&);

  void dump (const std::string&) const;
};

#endif
////////////////////////////////////////////////////////////////////////////////
