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
#include <CmdUDAs.h>
#include <sstream>
#include <algorithm>
#include <Context.h>
#include <Filter.h>
#include <ViewText.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdUDAs::CmdUDAs ()
{
  _keyword               = "udas";
  _usage                 = "task          udas";
  _description           = STRING_CMD_UDAS_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::config;
}

////////////////////////////////////////////////////////////////////////////////
int CmdUDAs::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  std::vector <std::string> udas;
  for (auto& name : context.config)
  {
    if (name.first.substr (0, 4) == "uda." &&
        name.first.find (".type") != std::string::npos)
    {
      auto period = name.first.find ('.', 4);
      if (period != std::string::npos)
        udas.push_back (name.first.substr (4, period - 4));
    }
  }

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  if (udas.size ())
  {
    std::sort (udas.begin (), udas.end ());

    // Render a list of UDA name, type, label, allowed values,
    // possible default value, and finally the usage count.
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_UDA));
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_TYPE));
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_LABEL));
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_VALUES));
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_DEFAULT));
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_UDACOUNT));

    if (context.color ())
    {
      Color label (context.config.get ("color.label"));
      view.colorHeader (label);
    }

    for (auto& uda : udas)
    {
      std::string type   = context.config.get ("uda." + uda + ".type");
      std::string label  = context.config.get ("uda." + uda + ".label");
      std::string values = context.config.get ("uda." + uda + ".values");
      std::string defval = context.config.get ("uda." + uda + ".default");
      if (label == "")
        label = uda;

      // Count UDA usage by UDA.
      int count = 0;
      for (auto& i : filtered)
        if (i.has (uda))
          ++count;

      int row = view.addRow ();
      view.set (row, 0, uda);
      view.set (row, 1, type);
      view.set (row, 2, label);
      view.set (row, 3, values);
      view.set (row, 4, defval);
      view.set (row, 5, count);
    }

    out << optionalBlankLine ()
        << view.render ()
        << optionalBlankLine ()
        << (udas.size () == 1
              ? format (STRING_CMD_UDAS_SUMMARY,  udas.size ())
              : format (STRING_CMD_UDAS_SUMMARY2, udas.size ()))
        << "\n";
  }
  else
  {
    out << STRING_CMD_UDAS_NO << "\n";
    rc = 1;
  }

  // Orphans are task attributes that are not represented in context.columns.
  std::map <std::string, int> orphans;
  for (auto& i : filtered)
  {
    for (auto& att : i.data)
      if (att.first.substr (0, 11) != "annotation_" &&
          context.columns.find (att.first) == context.columns.end ())
        orphans[att.first]++;
  }

  if (orphans.size ())
  {
    // Display the orphans and their counts.
    ViewText orphanView;
    orphanView.width (context.getWidth ());
    orphanView.add (Column::factory ("string", STRING_COLUMN_LABEL_ORPHAN));
    orphanView.add (Column::factory ("string", STRING_COLUMN_LABEL_UDACOUNT));

    if (context.color ())
    {
      Color label (context.config.get ("color.label"));
      orphanView.colorHeader (label);
    }

    for (auto& o : orphans)
    {
      int row = orphanView.addRow ();
      orphanView.set (row, 0, o.first);
      orphanView.set (row, 1, o.second);
    }

    out << optionalBlankLine ()
        << orphanView.render ()
        << optionalBlankLine ()
        << (udas.size () == 1
              ? format (STRING_CMD_UDAS_ORPHAN,  orphans.size ())
              : format (STRING_CMD_UDAS_ORPHANS, orphans.size ()))
        << "\n";
  }

  output = out.str ();
  return rc;
}

///////////////////////////////////////////////////////////////////////////////
CmdCompletionUDAs::CmdCompletionUDAs ()
{
  _keyword               = "_udas";
  _usage                 = "task          _udas";
  _description           = STRING_CMD_UDAS_COMPL_USAGE;
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
int CmdCompletionUDAs::execute (std::string& output)
{
  std::vector <std::string> udas;
  for (auto& name : context.config)
  {
    if (name.first.substr (0, 4) == "uda." &&
        name.first.find (".type") != std::string::npos)
    {
      auto period = name.first.find ('.', 4);
      if (period != std::string::npos)
        udas.push_back (name.first.substr (4, period - 4));
    }
  }

  if (udas.size ())
  {
    std::sort (udas.begin (), udas.end ());
    join (output, "\n", udas);
    output += "\n";
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
