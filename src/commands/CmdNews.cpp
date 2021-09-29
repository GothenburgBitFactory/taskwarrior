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
NewsItem::NewsItem (
  bool major,
  const std::string& title,
  const std::string& bg_title,
  const std::string& background,
  const std::string& punchline,
  const std::string& update,
  const std::string& reasoning,
  const std::string& actions
) {
  _major = major;
  _title = title;
  _bg_title = bg_title;
  _background = background;
  _punchline = punchline;
  _update = update;
  _reasoning = reasoning;
  _actions = actions;
}

void NewsItem::render () {
  auto config = Context::getContext ().config;
  Color header;
  Color footnote;
  Color bold;
  Color underline;
  if (Context::getContext ().color ()) {
    bold = Color ("bold");
    underline = Color ("underline");
    if (config.has ("color.header"))
      header = Color (config.get ("color.header"));
    if (config.has ("color.footnote"))
      footnote = Color (config.get ("color.footnote"));
  }

  // TODO: For some reason, bold cannot be blended in 256-color terminals
  // Apply this workaround of colorizing twice.
  std::cout << bold.colorize (header.colorize (format ("{1}\n", _title)));
  if (_background.size ()) {
    if (_bg_title.empty ())
      _bg_title = "Background";

    std::cout << "\n  " << underline.colorize (_bg_title) << std::endl
              << _background << std::endl;
  }

  wait_for_enter ();

  std::cout << "  " << underline.colorize ("What changed in 2.6.0?\n");
  if (_punchline.size ())
    std::cout << footnote.colorize (format ("{1}\n", _punchline));

  if (_update.size ())
    std::cout << format ("{1}\n", _update);

  wait_for_enter ();

  if (_reasoning.size ()) {
    std::cout << "  " << underline.colorize ("What is the motivation for this feature?\n")
              << _reasoning << std::endl;
    wait_for_enter ();
  }

  if (_actions.size ()) {
    std::cout << "  " << underline.colorize ("What do I have to do?\n")
              << _actions << std::endl;
    wait_for_enter ();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Generate the highlights for the 2.6.0 version.
//
// - XDG directory mode (high)
// - Support for Unicode 11 characters (high)
// - 64 bit values, UDAs, Datetime values until year 9999 (high)
// - Config context variables
// - Reports outside of context
// - Environment variables in taskrc (high)
// - Waiting is a virtual concept (high)
// - Improved parser and task display mechanism
// - The .by attribute modifier
// - Exporting a report
// - Multi-day holidays
void CmdNews::version2_6_0 (std::vector<NewsItem>& items) {
  /////////////////////////////////////////////////////////////////////////////
  // - Writeable context (major)

  // Detect whether user uses any contexts
  auto config = Context::getContext ().config;
  std::stringstream advice;

  auto defined = CmdContext::getContexts ();
  if (defined.size ())
  {
    // Detect the old-style contexts
    std::vector<std::string> old_style;
    std::copy_if (
      defined.begin(),
      defined.end(),
      std::back_inserter(old_style),
      [&](auto& name){return config.has ("context." + name);}
    );

    if (old_style.size ())
    {
      advice << format (
        "  You have {1} defined contexts, out of which {2} are old-style:\n",
        defined.size (),
        std::count_if (
          defined.begin (),
          defined.end (),
          [&](auto& name){return config.has ("context." + name);}
      ));

      for (auto context: defined) {
        std::string old_definition = config.get ("context." + context);
        if (old_definition != "")
          advice << format ("  * {1}: {2}\n", context, old_definition);
      }

      advice << "\n"
                "  These need to be migrated to new-style, which uses context.<name>.read and\n"
                "  context.<name>.write config variables. Please run the following commands:\n";

      for (auto context: defined) {
        std::string old_definition = config.get ("context." + context);
        if (old_definition != "")
          advice << format ("  $ task context define {1} '{2}'\n", context, old_definition);
      }

      advice << "\n"
                "  Please check these filters are also valid modifications. If a context filter is not\n"
                "  a valid modification, you can set the context.<name>.write configuration variable to\n"
                "  specify the write context explicitly. Read more in CONTEXT section of man taskrc.";
    }
    else
      advice << "  You don't have any old-style contexts defined, so you're good to go as is!";
  }
  else
    advice << "  You don't have any contexts defined, so you're good to go as is!\n"
              "  Read more about how to use contexts in CONTEXT section of 'man task'.";

  NewsItem writeable_context (
    true,
    "'Writeable' context",
    "Background - what is context?",
    "  The 'context' is a feature (introduced in 2.5.0) that allows users to apply a\n"
    "  predefined filter to all task reports.\n"
    "  \n"
    "    $ task context define work \"project:Work or +urgent\"\n"
    "    $ task context work\n"
    "    Context 'work' set. Use 'task context none' to remove.\n"
    "  \n"
    "  Now if we proceed to add two tasks:\n"
    "    $ task add Talk to Jeff pro:Work\n"
    "    $ task add Call mom pro:Personal\n"
    "  \n"
    "    $ task\n"
    "    ID Age   Project Description  Urg\n"
    "     1 16s   Work    Talk to Jeff    1\n"
    "  \n"
    "  The task \"Call mom\" will not be listed, because it does not match\n"
    "  the active context (its project is 'Personal' and not 'Work').",
    "  The currently active context definition is now applied as default modifications\n"
    "  when creating new tasks using 'task add' and 'task log'.",
    "  \n"
    "  Consider following example, using contex 'work' defined as 'project:Work' above:\n"
    "  \n"
    "    $ task context work\n"
    "    $ task add Talk to Jeff\n"
    "    $ task\n"
    "    ID Age  Project Description  Urg \n"
    "     1 1s   Work    Talk to Jeff    1\n"
    "            ^^^^^^^\n"
    "  \n"
    "  Note that project attribute was set to 'Work' automatically",
    "  This was a popular feature request. Now, if you have a context active,\n"
    "  newly added tasks no longer \"fall outside\" of the context by default.",
    advice.str ()
  );
  items.push_back(writeable_context);

  /////////////////////////////////////////////////////////////////////////////
  // - 64-bit datetime support (major)

  NewsItem uint64_support (
    false,
    "Support for 64-bit timestamps and numeric values",
    "",
    "",
    "  Taskwarrior now supports 64-bit timestamps, making it possible to set due dates\n"
    "  and other date attributes beyond 19 January 2038 (limit of 32-bit timestamps).\n",
    "  The current limit is 31 December 9999 for display reasons (last 4-digit year).",
    "  With each year passing by faster than the last, setting tasks for 2040s\n"
    "  is not as unfeasible as it once was.",
    "  Don't forget that 50-year anniversary and 'task add' a long-term task today!"
  );
  items.push_back(uint64_support);
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
