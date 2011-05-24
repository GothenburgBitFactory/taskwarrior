////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_VIEWTEXT
#define INCLUDED_VIEWTEXT

#include <string>
#include <vector>
#include <Task.h>
#include <Color.h>
#include <Column.h>

class ViewText
{
public:
  ViewText ();
  ~ViewText () {}

  // View specifications.
  void add (Column* col)          { _columns.push_back (col);    }
  void width (int width)          { _width = width;              }
  void leftMargin (int margin)    { _left_margin = margin;       }
  void colorHeader (Color& c)     { _header = c;                 }
  void colorOdd (Color& c)        { _odd = c;                    }
  void colorEven (Color& c)       { _even = c;                   }
  void intraPadding (int padding) { _intra_padding = padding;    }
  void intraColorOdd (Color& c)   { _intra_odd = c;              }
  void intraColorEven (Color& c)  { _intra_even = c;             }
  void extraPadding (int padding) { _extra_padding = padding;    }
  void extraColorOdd (Color& c)   { _extra_odd = c;              }
  void extraColorEven (Color& c)  { _extra_even = c;             }
  void truncateLines (int n)      { _truncate_lines = n;         }
  void truncateRows (int n)       { _truncate_rows = n;          }
  int lines ()                    { return _lines;               }
  int rows ()                     { return (int) _data.size ();  }

  // Data provision.
  int addRow ();
  void set (int, int, const std::string&, Color color = Color::nocolor);
  void set (int, int, int, Color color = Color::nocolor);
  void set (int, int, float, int, int, Color color = Color::nocolor);
  void set (int, int, Color);

  // View rendering.
  std::string render ();

private:
  std::vector <std::vector <std::string> > _data;
  std::vector <std::vector <Color> >       _color;
  std::vector <Column*>                    _columns;
  int                                      _width;
  int                                      _left_margin;
  Color                                    _header;
  Color                                    _odd;
  Color                                    _even;
  int                                      _intra_padding;
  Color                                    _intra_odd;
  Color                                    _intra_even;
  int                                      _extra_padding;
  Color                                    _extra_odd;
  Color                                    _extra_even;
  int                                      _truncate_lines;
  int                                      _truncate_rows;
  int                                      _lines;
  int                                      _rows;
};

#endif
////////////////////////////////////////////////////////////////////////////////

