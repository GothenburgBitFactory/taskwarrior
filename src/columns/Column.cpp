////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
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

#define L10N                                           // Localization complete.

#include <Context.h>
#include <Column.h>
#include <ColDepends.h>
#include <ColDescription.h>
#include <ColDue.h>
#include <ColEnd.h>
#include <ColEntry.h>
#include <ColID.h>
#include <ColPriority.h>
#include <ColProject.h>
#include <ColRecur.h>
#include <ColStart.h>
#include <ColStatus.h>
#include <ColString.h>
#include <ColTags.h>
#include <ColUntil.h>
#include <ColUrgency.h>
#include <ColUUID.h>
#include <ColWait.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Supports the complete column definition:
//
//   <type>[.<format>]
//
Column* Column::factory (const std::string& name, const std::string& report)
{
  // Decompose name into type and style.
  std::string::size_type dot = name.find ('.');
  std::string column_name;
  std::string column_style;
  if (dot != std::string::npos)
  {
    column_name = name.substr (0, dot);
    column_style = name.substr (dot + 1);
  }
  else
  {
    column_name = name;
    column_style = "default";
  }

  Column* c;
       if (column_name == "depends")     c = new ColumnDepends ();
  else if (column_name == "description") c = new ColumnDescription ();
  else if (column_name == "due")         c = new ColumnDue ();
  else if (column_name == "end")         c = new ColumnEnd ();
  else if (column_name == "entry")       c = new ColumnEntry ();
  else if (column_name == "id")          c = new ColumnID ();
  else if (column_name == "priority")    c = new ColumnPriority ();
  else if (column_name == "project")     c = new ColumnProject ();
  else if (column_name == "recur")       c = new ColumnRecur ();
  else if (column_name == "start")       c = new ColumnStart ();
  else if (column_name == "status")      c = new ColumnStatus ();
  else if (column_name == "tags")        c = new ColumnTags ();
  else if (column_name == "until")       c = new ColumnUntil ();
  else if (column_name == "urgency")     c = new ColumnUrgency ();
  else if (column_name == "uuid")        c = new ColumnUUID ();
  else if (column_name == "wait")        c = new ColumnWait ();

  // Special non-task column.
  else if (column_name == "string")      c = new ColumnString ();
  else
    throw format (STRING_COLUMN_BAD_NAME, column_name);

  c->setReport (report);
  c->setStyle (column_style);
  return c;
}

////////////////////////////////////////////////////////////////////////////////
// Bulk column instantiation.
void Column::factory (std::map <std::string, Column*>& all)
{
  Column* c;

  c = new ColumnDepends ();        all[c->_name] = c;
  c = new ColumnDescription ();    all[c->_name] = c;
  c = new ColumnDue ();            all[c->_name] = c;
  c = new ColumnEnd ();            all[c->_name] = c;
  c = new ColumnEntry ();          all[c->_name] = c;
  c = new ColumnID ();             all[c->_name] = c;
  c = new ColumnPriority ();       all[c->_name] = c;
  c = new ColumnProject ();        all[c->_name] = c;
  c = new ColumnRecur ();          all[c->_name] = c;
  c = new ColumnStart ();          all[c->_name] = c;
  c = new ColumnStatus ();         all[c->_name] = c;
  c = new ColumnTags ();           all[c->_name] = c;
  c = new ColumnUntil ();          all[c->_name] = c;
  c = new ColumnUrgency ();        all[c->_name] = c;
  c = new ColumnUUID ();           all[c->_name] = c;
  c = new ColumnWait ();           all[c->_name] = c;
}

////////////////////////////////////////////////////////////////////////////////
Column::Column ()
: _name ("")
, _type ("string")
, _style ("default")
, _label ("")
, _report ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Column::Column (const Column& other)
{
  _name  = other._name;
  _type  = other._type;
  _style = other._style;
  _label = other._label;
  _label = other._report;
}

////////////////////////////////////////////////////////////////////////////////
Column& Column::operator= (const Column& other)
{
  if (this != &other)
  {
    _name    = other._name;
    _type    = other._type;
    _style   = other._style;
    _label   = other._label;
    _report  = other._report;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Column::operator== (const Column& other) const
{
  return _name  == other._name  &&
         _type  == other._type  &&
         _style == other._style &&
         _label == other._label;
}

////////////////////////////////////////////////////////////////////////////////
Column::~Column ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Column::renderHeader (
  std::vector <std::string>& lines,
  int width,
  Color& color)
{
  if (context.verbose ("label") &&
     nontrivial (_label))
  {
    // Create a basic label.
    std::string header;
    header.reserve (width);
    header = _label;

    // Create a fungible copy.
    Color c = color;

    // Now underline the header, or add a dashed line.
    if (context.color () &&
        context.config.getBoolean ("fontunderline"))
    {
      c.blend (Color (Color::nocolor, Color::nocolor, true, false, false));
      lines.push_back (c.colorize (leftJustify (header, width)));
    }
    else
    {
      lines.push_back (c.colorize (leftJustify (header, width)));
      lines.push_back (c.colorize (std::string (width, '-')));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Column::validate (std::string& input)
{
  return input.length () ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// No L10N.
void Column::measure (const std::string&, int&, int&)
{
  throw std::string ("Virtual method Column::measure not overriden.");
}

////////////////////////////////////////////////////////////////////////////////
// No L10N.
void Column::measure (Task&, int&, int&)
{
  throw std::string ("Virtual method Column::measure not overriden.");
}

////////////////////////////////////////////////////////////////////////////////
// No L10N.
void Column::render (std::vector <std::string>&, const std::string&, int, Color&)
{
  throw std::string ("Virtual method Column::render not overriden.");
}

////////////////////////////////////////////////////////////////////////////////
// No L10N.
void Column::render (std::vector <std::string>&, Task&, int, Color&)
{
  throw std::string ("Virtual method Column::render not overriden.");
}

////////////////////////////////////////////////////////////////////////////////
