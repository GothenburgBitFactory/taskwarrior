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
#include <CmdReports.h>
#include <sstream>
#include <Context.h>
#include <Table.h>
#include <format.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
CmdReports::CmdReports ()
{
  _keyword               = "reports";
  _usage                 = "task          reports";
  _description           = "Lists all supported reports";
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
int CmdReports::execute (std::string& output)
{
  std::vector <std::string> reports;

  // Add custom reports.
  for (auto& i : Context::getContext ().config)
  {
    if (i.first.substr (0, 7) == "report.")
    {
      std::string report = i.first.substr (7);
      auto columns = report.find (".columns");
      if (columns != std::string::npos)
        reports.push_back (report.substr (0, columns));
    }
  }

  // Add known reports.
  reports.push_back ("burndown.daily");
  reports.push_back ("burndown.monthly");
  reports.push_back ("burndown.weekly");
  reports.push_back ("ghistory.annual");
  reports.push_back ("ghistory.monthly");
  reports.push_back ("history.annual");
  reports.push_back ("history.monthly");
  reports.push_back ("information");
  reports.push_back ("projects");
  reports.push_back ("summary");
  reports.push_back ("tags");

  std::sort (reports.begin (), reports.end ());

  // Compose the output.
  std::stringstream out;
  Table view;
  view.width (Context::getContext ().getWidth ());
  view.add ("Report");
  view.add ("Description");
  setHeaderUnderline (view);

  for (auto& report : reports)
  {
    int row = view.addRow ();
    view.set (row, 0, report);
    view.set (row, 1, Context::getContext ().commands[report]->description ());
  }

  out << optionalBlankLine ()
      << view.render ()
      << optionalBlankLine ()
      << format ("{1} reports\n", reports.size ());

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
