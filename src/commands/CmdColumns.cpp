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
#include <CmdColumns.h>
#include <algorithm>
#include <Context.h>
#include <ViewText.h>
#include <Color.h>
#include <text.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdColumns::CmdColumns ()
{
  _keyword               = "columns";
  _usage                 = "task          columns [substring]";
  _description           = STRING_CMD_COLUMNS_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::config;
}

////////////////////////////////////////////////////////////////////////////////
int CmdColumns::execute (std::string& output)
{
  // Obtain the arguments from the description.  That way, things like '--'
  // have already been handled.
  std::vector <std::string> words = context.cli2.getWords ();
  if (words.size () > 1)
    throw std::string (STRING_CMD_COLUMNS_ARGS);

  // Include all columns in the table.
  std::vector <std::string> names;
  for (auto& col : context.columns)
    names.push_back (col.first);

  std::sort (names.begin (), names.end ());

  // Render a list of column names, formats and examples.
  ViewText formats;
  formats.width (context.getWidth ());
  formats.add (Column::factory ("string", STRING_COLUMN_LABEL_COLUMN));
  formats.add (Column::factory ("string", STRING_COLUMN_LABEL_TYPE));
  formats.add (Column::factory ("string", STRING_COLUMN_LABEL_MODIFY));
  formats.add (Column::factory ("string", STRING_COLUMN_LABEL_STYLES));
  formats.add (Column::factory ("string", STRING_COLUMN_LABEL_EXAMPLES));

  if (context.color ())
  {
    Color label (context.config.get ("color.label"));
    formats.colorHeader (label);

    Color alternate (context.config.get ("color.alternate"));
    formats.colorOdd (alternate);
    formats.intraColorOdd (alternate);
  }

  for (auto& name : names)
  {
    if (words.size () == 0 ||
        find (name, words[0], false) != std::string::npos)
    {
      const std::vector <std::string> styles   = context.columns[name]->styles ();
      const std::vector <std::string> examples = context.columns[name]->examples ();

      for (unsigned int i = 0; i < styles.size (); ++i)
      {
        int row = formats.addRow ();
        formats.set (row, 0, i == 0 ? name : "");
        formats.set (row, 1, i == 0 ? context.columns[name]->type () : "");
        formats.set (row, 2, i == 0 ? (context.columns[name]->modifiable () ? STRING_COLUMN_LABEL_MODIFY : STRING_COLUMN_LABEL_NOMODIFY) : "");
        formats.set (row, 3, styles[i] + (i == 0 ? "*" : ""));
        formats.set (row, 4, i < examples.size () ? examples[i] : "");
      }
    }
  }

  int row = formats.addRow ();
  formats.set (row, 0, "<uda>");
  formats.set (row, 1, "<type>");
  formats.set (row, 2, "Modifiable");
  formats.set (row, 3, "default*");

  row = formats.addRow ();
  formats.set (row, 0, "");
  formats.set (row, 3, "indicator");

  output = optionalBlankLine ()
         + formats.render ()
         + "\n"
         + STRING_CMD_COLUMNS_NOTE
         + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionColumns::CmdCompletionColumns ()
{
  _keyword               = "_columns";
  _usage                 = "task          _columns";
  _description           = STRING_CMD_COLUMNS_USAGE2;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionColumns::execute (std::string& output)
{
  // Include all columns.
  std::vector <std::string> names;
  for (auto& col : context.columns)
    names.push_back (col.first);

  std::sort (names.begin (), names.end ());

  // Render only the column names.
  for (auto& name : names)
    output += name + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
