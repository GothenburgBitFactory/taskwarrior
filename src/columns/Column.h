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

#ifndef INCLUDED_COLUMN
#define INCLUDED_COLUMN

#include <vector>
#include <string>
#include <Color.h>
#include <Task.h>

class Column
{
public:
  static Column* factory (const std::string&, const std::string&);
  static void factory (std::map <std::string, Column*>&);
  static void uda (std::map <std::string, Column*>&);
  static Column* uda (const std::string&);

  Column ();
  virtual ~Column ();

  const std::string& name () const            { return _name;        }
  const std::string& style () const           { return _style;       }
  const std::string& label () const           { return _label;       }
  const std::string& type () const            { return _type;        }
  bool modifiable () const                    { return _modifiable;  }
  bool is_uda () const                        { return _uda;         }
  bool is_fixed_width () const                { return _fixed_width; }
  std::vector <std::string> styles () const   { return _styles;      }
  std::vector <std::string> examples () const { return _examples;    }

  virtual void setStyle  (const std::string&);
  virtual void setLabel  (const std::string& value) { _label = value;  }
  virtual void setReport (const std::string& value) { _report = value; }

  virtual bool validate (const std::string&) const;
  virtual void measure (const std::string&, unsigned int&, unsigned int&)           {};
  virtual void measure (Task&, unsigned int&, unsigned int&)                        {};
  virtual void renderHeader (std::vector <std::string>&, int, Color&);
  virtual void render (std::vector <std::string>&, const std::string&, int, Color&) {};
  virtual void render (std::vector <std::string>&, Task&, int, Color&)              {};
  virtual void modify (Task&, const std::string&)                                   {};

protected:
  void renderInteger     (std::vector <std::string>&, int, Color&, int);
  void renderDouble      (std::vector <std::string>&, int, Color&, double);
  void renderStringLeft  (std::vector <std::string>&, int, Color&, const std::string&);
  void renderStringRight (std::vector <std::string>&, int, Color&, const std::string&);

protected:
  std::string _name;
  std::string _type;
  std::string _style;
  std::string _label;
  std::string _report;
  bool _modifiable;
  bool _uda;
  bool _fixed_width;
  std::vector <std::string> _styles;
  std::vector <std::string> _examples;
};

#endif
////////////////////////////////////////////////////////////////////////////////
