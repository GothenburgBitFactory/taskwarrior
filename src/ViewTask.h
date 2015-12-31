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

#ifndef INCLUDED_VIEWTASK
#define INCLUDED_VIEWTASK

#include <string>
#include <vector>
#include <Task.h>
#include <Color.h>
#include <Column.h>

class ViewTask
{
public:
  ViewTask ();
  ~ViewTask ();

  // View specifications.
  void add (Column* column, bool sort = false) { _columns.push_back (column); _sort.push_back (sort); }
  void width (int width)                       { _width = width;                                      }
  void leftMargin (int margin)                 { _left_margin = margin;                               }
  void colorHeader (Color& c)                  { _header = c; if (!_sort_header) _sort_header = c;    }
  void colorSortHeader (Color& c)              { _sort_header = c;                                    }
  void colorOdd (Color& c)                     { _odd = c;                                            }
  void colorEven (Color& c)                    { _even = c;                                           }
  void intraPadding (int padding)              { _intra_padding = padding;                            }
  void intraColorOdd (Color& c)                { _intra_odd = c;                                      }
  void intraColorEven (Color& c)               { _intra_even = c;                                     }
  void extraPadding (int padding)              { _extra_padding = padding;                            }
  void extraColorOdd (Color& c)                { _extra_odd = c;                                      }
  void extraColorEven (Color& c)               { _extra_even = c;                                     }
  void truncateLines (int n)                   { _truncate_lines = n;                                 }
  void truncateRows (int n)                    { _truncate_rows = n;                                  }
  void addBreak (const std::string& attr)      { _breaks.push_back (attr);                            }
  int lines ()                                 { return _lines;                                       }
  int rows ()                                  { return _rows;                                        }

  // View rendering.
  std::string render (std::vector <Task>&, std::vector <int>&);

private:
  std::vector <Column*>     _columns;
  std::vector <bool>        _sort;
  std::vector <std::string> _breaks;
  int                       _width;
  int                       _left_margin;
  Color                     _header;
  Color                     _sort_header;
  Color                     _odd;
  Color                     _even;
  int                       _intra_padding;
  Color                     _intra_odd;
  Color                     _intra_even;
  int                       _extra_padding;
  Color                     _extra_odd;
  Color                     _extra_even;
  int                       _truncate_lines;
  int                       _truncate_rows;
  int                       _lines;
  int                       _rows;
};

#endif
////////////////////////////////////////////////////////////////////////////////

