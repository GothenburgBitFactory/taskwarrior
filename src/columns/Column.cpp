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

#include <cmake.h>
#include <Column.h>
#include <algorithm>
#include <set>
#include <Context.h>
#include <ColDepends.h>
#include <ColDescription.h>
#include <ColDue.h>
#include <ColEnd.h>
#include <ColEntry.h>
#include <ColID.h>
#include <ColIMask.h>
#include <ColMask.h>
#include <ColModified.h>
#include <ColParent.h>
#include <ColProject.h>
#include <ColRecur.h>
#include <ColScheduled.h>
#include <ColStart.h>
#include <ColStatus.h>
#include <ColString.h>
#include <ColTags.h>
#include <ColUntil.h>
#include <ColUrgency.h>
#include <ColUUID.h>
#include <ColUDA.h>
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
  auto dot = name.find ('.');
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
  else if (column_name == "imask")       c = new ColumnIMask ();
  else if (column_name == "mask")        c = new ColumnMask ();
  else if (column_name == "modified")    c = new ColumnModified ();
  else if (column_name == "parent")      c = new ColumnParent ();
  else if (column_name == "project")     c = new ColumnProject ();
  else if (column_name == "recur")       c = new ColumnRecur ();
  else if (column_name == "scheduled")   c = new ColumnScheduled ();
  else if (column_name == "start")       c = new ColumnStart ();
  else if (column_name == "status")      c = new ColumnStatus ();
  else if (column_name == "tags")        c = new ColumnTags ();
  else if (column_name == "until")       c = new ColumnUntil ();
  else if (column_name == "urgency")     c = new ColumnUrgency ();
  else if (column_name == "uuid")        c = new ColumnUUID ();
  else if (column_name == "wait")        c = new ColumnWait ();

  // Special non-task column.
  else if (column_name == "string")      c = new ColumnString ();

  // UDA.
  else if (context.config.has ("uda." + column_name + ".type"))
    c = Column::uda (column_name);

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
  c = new ColumnIMask ();          all[c->_name] = c;
  c = new ColumnMask ();           all[c->_name] = c;
  c = new ColumnModified ();       all[c->_name] = c;
  c = new ColumnParent ();         all[c->_name] = c;
  c = new ColumnProject ();        all[c->_name] = c;
  c = new ColumnRecur ();          all[c->_name] = c;
  c = new ColumnScheduled ();      all[c->_name] = c;
  c = new ColumnStart ();          all[c->_name] = c;
  c = new ColumnStatus ();         all[c->_name] = c;
  c = new ColumnTags ();           all[c->_name] = c;
  c = new ColumnUntil ();          all[c->_name] = c;
  c = new ColumnUrgency ();        all[c->_name] = c;
  c = new ColumnUUID ();           all[c->_name] = c;
  c = new ColumnWait ();           all[c->_name] = c;

  Column::uda (all);
}

////////////////////////////////////////////////////////////////////////////////
void Column::uda (std::map <std::string, Column*>& all)
{
  // For each UDA, instantiate and initialize ColumnUDA().
  std::set <std::string> udas;

  for (const auto& i : context.config)
  {
    if (i.first.substr (0, 4) == "uda.")
    {
      std::string::size_type period = 4; // One byte after the first '.'.

      if ((period = i.first.find ('.', period)) != std::string::npos)
        udas.insert (i.first.substr (4, period - 4));
    }
  }

  for (const auto& uda : udas)
  {
    if (all.find (uda) != all.end ())
      throw format (STRING_UDA_COLLISION, uda);

    Column* c = Column::uda (uda);
    if (c)
      all[c->_name] = c;
  }
}

////////////////////////////////////////////////////////////////////////////////
Column* Column::uda (const std::string& name)
{
  auto type   = context.config.get ("uda." + name + ".type");
  auto label  = context.config.get ("uda." + name + ".label");
  auto values = context.config.get ("uda." + name + ".values");

  if (type == "string")
  {
    auto c = new ColumnUDAString ();
    c->_name = name;
    c->_label = label;
    if (values != "")
      split (c->_values, values, ',');
    return c;
  }
  else if (type == "numeric")
  {
    auto c = new ColumnUDANumeric ();
    c->_name = name;
    c->_label = label;
    if (values != "")
      split (c->_values, values, ',');
    return c;
  }
  else if (type == "date")
  {
    auto c = new ColumnUDADate ();
    c->_name = name;
    c->_label = label;
    if (values != "")
      split (c->_values, values, ',');
    return c;
  }
  else if (type == "duration")
  {
    auto c = new ColumnUDADuration ();
    c->_name = name;
    c->_label = label;
    if (values != "")
      split (c->_values, values, ',');
    return c;
  }
  else if (type != "")
    throw std::string (STRING_UDA_TYPE);

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
Column::Column ()
: _name ("")
, _type ("string")
, _style ("default")
, _label ("")
, _report ("")
, _modifiable (true)
, _uda (false)
, _fixed_width (false)
{
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
      _label != "")
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
void Column::setStyle (const std::string& style)
{
  if (style != "default" &&
      std::find (_styles.begin (), _styles.end (), style) == _styles.end ())
    throw format (STRING_COLUMN_BAD_FORMAT, _name, style);

  _style = style;
}

////////////////////////////////////////////////////////////////////////////////
bool Column::validate (const std::string& input) const
{
  return input.length () ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// All integer values are right-justified.
void Column::renderInteger (
  std::vector <std::string>& lines,
  int width,
  Color& color,
  int value)
{
  lines.push_back (
    color.colorize (
      rightJustify (value, width)));
}

////////////////////////////////////////////////////////////////////////////////
// All floating point values are right-justified.
void Column::renderDouble (
  std::vector <std::string>& lines,
  int width,
  Color& color,
  double value)
{
  lines.push_back (
    color.colorize (
      rightJustify (
        format (value, 4, 3), width)));
}

////////////////////////////////////////////////////////////////////////////////
void Column::renderStringLeft (
  std::vector <std::string>& lines,
  int width,
  Color& color,
  const std::string& value)
{
  lines.push_back (
    color.colorize (
      leftJustify (value, width)));
}

////////////////////////////////////////////////////////////////////////////////
void Column::renderStringRight (
  std::vector <std::string>& lines,
  int width,
  Color& color,
  const std::string& value)
{
  lines.push_back (
    color.colorize (
      rightJustify (value, width)));
}

////////////////////////////////////////////////////////////////////////////////
