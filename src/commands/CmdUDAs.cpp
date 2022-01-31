////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <CmdUDAs.h>
#include <Table.h>
#include <sstream>
#include <algorithm>
#include <Context.h>
#include <Filter.h>
#include <main.h>
#include <format.h>
#include <shared.h>
#include <util.h>
#include <Task.h>

////////////////////////////////////////////////////////////////////////////////
CmdUDAs::CmdUDAs ()
{
  _keyword               = "udas";
  _usage                 = "task          udas";
  _description           = "Shows all the defined UDA details";
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
  for (auto& name : Context::getContext ().config)
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
    Table table;
    table.width (Context::getContext ().getWidth ());
    table.add ("Name");
    table.add ("Type");
    table.add ("Label");
    table.add ("Allowed Values");
    table.add ("Default");
    table.add ("Usage Count");
    setHeaderUnderline (table);

    for (auto& uda : udas)
    {
      std::string type   = Context::getContext ().config.get ("uda." + uda + ".type");
      std::string label  = Context::getContext ().config.get ("uda." + uda + ".label");
      std::string values = Context::getContext ().config.get ("uda." + uda + ".values");
      std::string defval = Context::getContext ().config.get ("uda." + uda + ".default");
      if (label == "")
        label = uda;

      // Count UDA usage by UDA.
      int count = 0;
      for (auto& i : filtered)
        if (i.has (uda))
          ++count;

      int row = table.addRow ();
      table.set (row, 0, uda);
      table.set (row, 1, type);
      table.set (row, 2, label);
      table.set (row, 3, values);
      table.set (row, 4, defval);
      table.set (row, 5, count);
    }

    out << optionalBlankLine ()
        << table.render ()
        << optionalBlankLine ()
        << (udas.size () == 1
              ? format ("{1} UDA defined", udas.size ())
              : format ("{1} UDAs defined", udas.size ()))
        << '\n';
  }
  else
  {
    out << "No UDAs defined.\n";
    rc = 1;
  }

  // Orphans are task attributes that are not represented in context.columns.
  std::map <std::string, int> orphans;
  for (auto& i : filtered)
  {
    for (auto& att : i.getUDAOrphans ())
      orphans[att]++;
  }

  if (orphans.size ())
  {
    // Display the orphans and their counts.
    Table orphanTable;
    orphanTable.width (Context::getContext ().getWidth ());
    orphanTable.add ("Orphan UDA");
    orphanTable.add ("Usage Count");
    setHeaderUnderline (orphanTable);

    for (auto& o : orphans)
    {
      int row = orphanTable.addRow ();
      orphanTable.set (row, 0, o.first);
      orphanTable.set (row, 1, o.second);
    }

    out << optionalBlankLine ()
        << orphanTable.render ()
        << optionalBlankLine ()
        << (udas.size () == 1
              ? format ("{1} Orphan UDA", orphans.size ())
              : format ("{1} Orphan UDAs", orphans.size ()))
        << '\n';
  }

  output = out.str ();
  return rc;
}

///////////////////////////////////////////////////////////////////////////////
CmdCompletionUDAs::CmdCompletionUDAs ()
{
  _keyword               = "_udas";
  _usage                 = "task          _udas";
  _description           = "Shows the defined UDAs for completion purposes";
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
  for (auto& name : Context::getContext ().config)
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
    output = join ("\n", udas) + '\n';
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
