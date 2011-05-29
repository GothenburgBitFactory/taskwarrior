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

#include <ViewText.h>
#include <Context.h>
#include <Timer.h>
#include <text.h>
#include <utf8.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ViewText::ViewText ()
: _width (0)
, _left_margin (0)
, _header (0)
, _odd (0)
, _even (0)
, _intra_padding (1)
, _intra_odd (0)
, _intra_even (0)
, _extra_padding (0)
, _extra_odd (0)
, _extra_even (0)
, _truncate_lines (0)
, _truncate_rows (0)
, _lines (0)
, _rows (0)
{
}

////////////////////////////////////////////////////////////////////////////////
int ViewText::addRow ()
{
  _data.push_back (std::vector <std::string> (_columns.size (), ""));
  _color.push_back (std::vector <Color> (_columns.size (), Color::nocolor));
  return _data.size () - 1;
}

////////////////////////////////////////////////////////////////////////////////
void ViewText::set (int row, int col, const std::string& value, Color color)
{
  _data[row][col] = value;

  if (color.nontrivial () &&
      context.color ())
    _color[row][col] = color;
}

////////////////////////////////////////////////////////////////////////////////
void ViewText::set (int row, int col, int value, Color color)
{
  std::string string_value = format (value);
  _data[row][col] = string_value;

  if (color.nontrivial () &&
      context.color ())
    _color[row][col] = color;
}

////////////////////////////////////////////////////////////////////////////////
void ViewText::set (int row, int col, float value, int width, int precision, Color color)
{
  std::string string_value = format ((float)value, width, precision);
  _data[row][col] = string_value;

  if (color.nontrivial () &&
      context.color ())
    _color[row][col] = color;
}

////////////////////////////////////////////////////////////////////////////////
void ViewText::set (int row, int col, Color color)
{
  if (color.nontrivial () &&
      context.color ())
    _color[row][col] = color;
}

////////////////////////////////////////////////////////////////////////////////
std::string ViewText::render ()
{
  Timer timer ("ViewText::render");

  // Determine minimal, ideal column widths.
  std::vector <int> minimal;
  std::vector <int> ideal;
  for (unsigned int col = 0; col < _columns.size (); ++col)
  {
    // Headers factor in to width calculations.
    int global_min = utf8_length (_columns[col]->getLabel ());
    int global_ideal = global_min;

    for (unsigned int row = 0; row < _data.size (); ++row)
    {
      // Determine minimum and ideal width for this column.
      int min;
      int ideal;
      _columns[col]->measure (_data[row][col], min, ideal);

      if (min   > global_min)   global_min = min;
      if (ideal > global_ideal) global_ideal = ideal;
    }

    minimal.push_back (global_min);
    ideal.push_back (global_ideal);
  }

  // Sum the minimal widths.
  int sum_minimal = 0;
  std::vector <int>::iterator c;
  for (c = minimal.begin (); c != minimal.end (); ++c)
    sum_minimal += *c;

  // Sum the ideal widths.
  int sum_ideal = 0;
  for (c = ideal.begin (); c != ideal.end (); ++c)
    sum_ideal += *c;

  // Calculate final column widths.
  int overage = _width
              - _left_margin
              - (2 * _extra_padding)
              - ((_columns.size () - 1) * _intra_padding);

  std::vector <int> widths;
  if (sum_ideal <= overage)
    widths = ideal;
  else if (sum_minimal > overage)
//    throw std::string ("There is not enough horizontal width to display the results.");
    widths = minimal;
  else
  {
    widths = minimal;
    overage -= sum_minimal;

    // Spread 'overage' among columns where width[i] < ideal[i]
    while (overage)
    {
      for (unsigned int i = 0; i < _columns.size () && overage; ++i)
      {
        if (widths[i] < ideal[i])
        {
          ++widths[i];
          --overage;
        }
      }
    }
  }

  // Compose column headers.
  unsigned int max_lines = 0;
  std::vector <std::vector <std::string> > headers;
  for (unsigned int c = 0; c < _columns.size (); ++c)
  {
    headers.push_back (std::vector <std::string> ());
    _columns[c]->renderHeader (headers[c], widths[c], _header);

    if (headers[c].size () > max_lines)
      max_lines = headers[c].size ();
  }

  // Output string.
  std::string out;
  _lines = 0;

  // Render column headers.
  std::string left_margin = std::string (_left_margin, ' ');
  std::string extra       = std::string (_extra_padding, ' ');
  std::string intra       = std::string (_intra_padding, ' ');

  std::string extra_odd   = context.color () ? _extra_odd.colorize  (extra) : extra;
  std::string extra_even  = context.color () ? _extra_even.colorize (extra) : extra;
  std::string intra_odd   = context.color () ? _intra_odd.colorize  (intra) : intra;
  std::string intra_even  = context.color () ? _intra_even.colorize (intra) : intra;

  for (unsigned int i = 0; i < max_lines; ++i)
  {
    out += left_margin + extra;

    for (unsigned int c = 0; c < _columns.size (); ++c)
    {
      if (c)
        out += intra;

      if (headers[c].size () < max_lines - i)
        out += _header.colorize (std::string (widths[c], ' '));
      else
        out += headers[c][i];
    }

    out += extra;

    // Trim right.
    out.erase (out.find_last_not_of (" ") + 1);
    out += "\n";

    // Stop if the line limit is exceeded.
    if (++_lines >= _truncate_lines && _truncate_lines != 0)
      return out;
  }

  // Compose, render columns, in sequence.
  _rows = 0;
  std::vector <std::vector <std::string> > cells;
  for (unsigned int row = 0; row < _data.size (); ++row)
  {
    max_lines = 0;

    // Alternate rows based on |s % 2|
    bool odd = (row % 2) ? true : false;
    Color row_color = odd ? _odd : _even;

    // TODO row_color.blend (provided color);
    // TODO Problem: colors for columns are specified, not rows,
    //      therefore there are only cell colors, not intra colors.

    Color cell_color;
    for (unsigned int col = 0; col < _columns.size (); ++col)
    {
      if (context.color ())
      {
        cell_color = row_color;
        cell_color.blend (_color[row][col]);
      }

      cells.push_back (std::vector <std::string> ());
      _columns[col]->render (cells[col], _data[row][col], widths[col], cell_color);

      if (cells[col].size () > max_lines)
        max_lines = cells[col].size ();
    }

    for (unsigned int i = 0; i < max_lines; ++i)
    {
      out += left_margin + (odd ? extra_odd : extra_even);

      for (unsigned int col = 0; col < _columns.size (); ++col)
      {
        if (col)
        {
          if (row_color.nontrivial ())
            out += row_color.colorize (intra);
          else
            out += (odd ? intra_odd : intra_even);
        }

        if (i < cells[col].size ())
          out += cells[col][i];
        else
        {
          if (context.color ())
          {
            cell_color = row_color;
            cell_color.blend (_color[row][col]);

            out += cell_color.colorize (std::string (widths[col], ' '));
          }
          else
            out += std::string (widths[col], ' ');
        }
      }

      out += (odd ? extra_odd : extra_even);

      // Trim right.
      out.erase (out.find_last_not_of (" ") + 1);
      out += "\n";

      // Stop if the line limit is exceeded.
      if (++_lines >= _truncate_lines && _truncate_lines != 0)
        return out;
    }

    cells.clear ();

    // Stop if the row limit is exceeded.
    if (++_rows >= _truncate_rows && _truncate_rows != 0)
      return out;
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
