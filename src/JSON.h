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

#ifndef INCLUDED_JSON
#define INCLUDED_JSON

#include <map>
#include <vector>
#include <string>
#include <Nibbler.h>

namespace json
{
  enum jtype
  {
    j_value,    // 0
    j_object,   // 1
    j_array,    // 2
    j_string,   // 3
    j_number,   // 4
    j_literal   // 5
  };

  class value
  {
  public:
    value () {}
    virtual ~value () {}
    static value* parse (Nibbler&);
    virtual jtype type ();
    virtual std::string dump () const;
  };

  class string : public value
  {
  public:
    string () {}
    string (const std::string&);
    ~string () {}
    static string* parse (Nibbler&);
    jtype type ();
    std::string dump () const;

  public:
    std::string _data;
  };

  class number : public value
  {
  public:
    number () : _dvalue (0.0) {}
    ~number () {}
    static number* parse (Nibbler&);
    jtype type ();
    std::string dump () const;
    operator double () const;

  public:
    double _dvalue;
  };

  class literal : public value
  {
  public:
    literal () : _lvalue (none) {}
    ~literal () {}
    static literal* parse (Nibbler&);
    jtype type ();
    std::string dump () const;

  public:
    enum literal_value {none, nullvalue, falsevalue, truevalue};
    literal_value _lvalue;
  };

  class array : public value
  {
  public:
    array () {}
    ~array ();
    static array* parse (Nibbler&);
    jtype type ();
    std::string dump () const;

  public:
    std::vector <value*> _data;
  };

  class object : public value
  {
  public:
    object () {}
    ~object ();
    static object* parse (Nibbler&);
    static bool parse_pair (Nibbler&, std::string&, value*&);
    jtype type ();
    std::string dump () const;

  public:
    std::map <std::string, value*> _data;
  };

  // Parser entry point.
  value* parse (const std::string&);

  // Encode/decode for JSON entities.
  std::string encode (const std::string&);
  std::string decode (const std::string&);
}

#endif
////////////////////////////////////////////////////////////////////////////////
