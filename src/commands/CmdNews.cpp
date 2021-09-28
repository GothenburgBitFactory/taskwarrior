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
#include <CmdNews.h>
#include <iostream>
#include <csignal>
#include <Table.h>
#include <Context.h>
#include <shared.h>
#include <format.h>
#include <util.h>
#include <main.h>

////////////////////////////////////////////////////////////////////////////////
CmdNews::CmdNews ()
{
  _keyword               = "news";
  _usage                 = "task          news";
  _description           = "Displays news about the recent releases";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::misc;
}

////////////////////////////////////////////////////////////////////////////////
static void signal_handler (int s)
{
  if (s == SIGINT)
  {
    std::cout << "\n\nCome back and read about new features later!\n";
    exit (1);
  }
}

void wait_for_enter ()
{
  signal (SIGINT, signal_handler);

  std::string dummy;
  std::cout << "\nPress enter to continue..";
  std::getline (std::cin, dummy);
  std::cout << "\33[2K\033[A\33[2K";  // Erase current line, move up, and erase again

  signal (SIGINT, SIG_DFL);
}

////////////////////////////////////////////////////////////////////////////////
// Holds information about single improvement / bug.
//
NewsItem::NewsItem (bool major, const std::string& title, const std::string& update) {
  _major = major;
  _title = title;
  _update = update;
}

void NewsItem::render () {
  auto config = Context::getContext ().config;
  Color header;
  Color bold;
  if (Context::getContext ().color ()) {
    bold = Color ("bold");
    if (config.has ("color.header"))
      header = Color (config.get ("color.header"));
  }

  // TODO: For some reason, bold cannot be blended in 256-color terminals
  // Apply this workaround of colorizing twice.
  std::cout << bold.colorize (header.colorize (format ("{1}\n", _title)));
  std::cout << format ("\n{1}\n", _update);
}

////////////////////////////////////////////////////////////////////////////////
// Generate the highlights for the 2.6.0 version.
//
// - XDG directory mode (high)
// - Support for Unicode 11 characters (high)
// - 64 bit values, UDAs, Datetime values until year 9999 (high)
// - Writeable context (high)
// - Config context variables
// - Reports outside of context
// - Environment variables in taskrc (high)
// - Waiting is a virtual concept (high)
// - Improved parser and task display mechanism
// - The .by attribute modifier
// - Exporting a report
// - Multi-day holidays
void CmdNews::version2_6_0 (std::vector<NewsItem>& items) {
  NewsItem writeable_context (
    true,
    "'Writeable' context",
    "  The currently active context definition is now applied as default\n"
    "  modifications with 'task add' and 'task log'."
  );
  items.push_back(writeable_context);
}

////////////////////////////////////////////////////////////////////////////////
int CmdNews::execute (std::string& output)
{
  auto words = Context::getContext ().cli2.getWords ();
  auto config = Context::getContext ().config;

  // TODO: 2.6.0 is the only version with explicit release notes, but in the
  // future we need to only execute yet unread release notes
  std::vector<NewsItem> items;
  version2_6_0 (items);

  // Determine whether to use full or short summary
  std::vector <std::string> options {"full", "short"};
  std::vector <std::string> matches;

  signal (SIGINT, signal_handler);

  do
  {
    std::cout << format (
      "Do you want to see full summary ({1} feature(s)) or a short one ({2} feature(s))?  (full/short) ",
      items.size (),
      std::count_if (items.begin (), items.end (), [](const NewsItem& n){return n._major;})
    );

    std::string answer;
    std::getline (std::cin, answer);
    answer = std::cin.eof () ? "full" : lowerCase (trim (answer));

    autoComplete (answer, options, matches, 1); // Hard-coded 1.
  }
  while (! std::cin.eof () && matches.size () != 1);

  signal (SIGINT, SIG_DFL);
  bool full_summary = matches.size () == 1 && matches[0] == "full" ? true : false;

  // Remove non-major items if displaying a non-full (abbreviated) summary
  if (! full_summary)
    items.erase (
      std::remove_if (items.begin (), items.end (), [](const NewsItem& n){return n._major == false;}),
      items.end ()
    );

  // Print release notes
  Color bold = Color ("bold");
  std::cout << bold.colorize (
    "\n"
    "===============================\n"
    "Taskwarrior 2.6.0 Release Notes\n"
    "===============================\n"
  );

  for (unsigned short i=0; i < items.size (); i++) {
    std::cout << format ("\n({1}/{2}) ", i+1, items.size ());
    items[i].render ();
    wait_for_enter ();
  }

  // Set a mark in the config to remember which version's release notes were displayed
  if (config.get ("news.version") == "2.6.0")
    output = "Repetition is the mother of all learning!\n";
  else {
    CmdConfig::setConfigVariable ("news.version", "2.6.0", false);
    output = "Thank you for catching up on the new features!\n";
  }

  return 0;
}
