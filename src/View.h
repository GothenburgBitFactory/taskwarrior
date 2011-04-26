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
#ifndef INCLUDED_VIEW
#define INCLUDED_VIEW

#include <string>
#include <vector>
#include <Task.h>
#include <Color.h>
#include <Column.h>

class View
{
public:
  View ();
  ~View ();

  // View specifications.
  void add (Column*);
  void width (int);
  void leftMargin (int);
  void colorOdd (Color&);
  void colorEven (Color&);
  void intraPadding (int);
  void extraPadding (int);
  void intraColorOdd (Color&);
  void intraColorEven (Color&);
  void extraColorOdd (Color&);
  void extraColorEven (Color&);
  void truncate (int);
  int lines ();

  // View rendering.
  std::string render (std::vector <Task>&, std::vector <int>&);

private:
  std::vector <Column*> _columns;
  int _width;
  int _left_margin;
  Color _odd;
  Color _even;
  int _intra_padding;
  Color _intra_odd;
  Color _intra_even;
  int _extra_padding;
  Color _extra_odd;
  Color _extra_even;
  int _truncate;
  int _lines;
};

#endif
////////////////////////////////////////////////////////////////////////////////

