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
#include <cmath>
#include <csignal>
#include <Table.h>
#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
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
    Color footnote;
    if (Context::getContext ().color ()) {
      if (Context::getContext ().config.has ("color.footnote"))
        footnote = Color (Context::getContext ().config.get ("color.footnote"));
    }

    std::cout << "\n\nCome back and read about new features later!\n";

    std::cout << footnote.colorize (
      "\nIf you enjoy Taskwarrior, please consider supporting the project at:\n"
      "    https://github.com/sponsors/GothenburgBitFactory/\n"
    );
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
    std::cout << "  " << underline.colorize ("What was the motivation behind this feature?\n")
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
    "  Note that project attribute was set to 'Work' automatically.",
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

  /////////////////////////////////////////////////////////////////////////////
  // - Waiting is a virtual status

  NewsItem waiting_status (
    true,
    "Deprecation of the status:waiting",
    "",
    "  If a task has a 'wait' attribute set to a date in the future, it is modified\n"
    "  to have a 'waiting' status. Once that date is no longer in the future, the status\n"
    "  is modified to back to 'pending'.",
    "  The 'waiting' value of status is deprecated, instead users should use +WAITING\n"
    "  virtual tag, or explicitly query for wait.after:now (the two are equivalent).",
    "  \n"
    "  The status:waiting query still works in 2.6.0, but support will be dropped in 3.0.",
    "",
    "  In your custom report definitions, the following expressions should be replaced:\n"
    "  * 'status:pending or status:waiting' should be replaced by 'status:pending'\n"
    "  * 'status:pending' should be replaced by 'status:pending -WAITING'"
  );
  items.push_back(waiting_status);

  /////////////////////////////////////////////////////////////////////////////
  // - Support for environment variables in the taskrc

  NewsItem env_vars (
    true,
    "Environment variables in the taskrc",
    "",
    "",
    "  Taskwarrior now supports expanding environment variables in the taskrc file,\n"
    "  allowing users to customize the behaviour of 'task' based on the current env.\n",
    "  The environment variables can either be used in paths, or as separate values:\n"
    "    data.location=$XDG_DATA_HOME/task/\n"
    "    default.project=$PROJECT",
    "",
    ""
  );
  items.push_back(env_vars);

  /////////////////////////////////////////////////////////////////////////////
  // - Reports outside of context

  NewsItem contextless_reports (
    true,
    "Context-less reports",
    "",
    "  By default, every report is affected by currently active context.",
    "  You can now make a selected report ignore currently active context by setting\n"
    "  'report.<name>.context' configuration variable to 0.",
    "",
    "  This is useful for users who utilize a single place (such as project:Inbox)\n"
    "  to collect their new tasks that are then triaged on a regular basis\n"
    "  (such as in GTD methodology).\n"
    "  \n"
    "  In such a case, defining a report that filters for project:Inbox and making it\n"
    "  fully accessible from any context is a major usability improvement.",
    ""
  );
  items.push_back(contextless_reports);

  /////////////////////////////////////////////////////////////////////////////
  // - Exporting a particular report

  NewsItem exportable_reports (
    false,
    "Exporting a particular report",
    "",
    "",
    "  You can now export the tasks listed by a particular report as JSON by simply\n"
    "  calling 'task export <report>'.\n",
    "  The export mirrors the filter and the sort order of the report.",
    "  This feature can be used to quickly process the data displayed in a particular\n"
    "  report using other CLI tools. For example, the following oneliner\n"
    "  \n"
    "      $ task export next | jq '.[].urgency' | datamash mean 1\n"
    "      3.3455535142857\n"
    "  \n"
    "  combines jq and GNU datamash to compute average urgency of the tasks displayed\n"
    "  in the 'next' report.",
    ""
  );
  items.push_back(exportable_reports);

  /////////////////////////////////////////////////////////////////////////////
  // - Multi-day holidays

  NewsItem multi_holidays (
    false,
    "Multi-day holidays",
    "",
    "  Holidays are currently used in 'task calendar' to visualize the workload during\n"
    "  the upcoming weeks/months. Up to date country-specific holiday data files can be\n"
    "  obtained from our website, holidata.net.",
    "  Instead of single-day holiday entries only, Taskwarrior now supports holidays\n"
    "  that span a range of days (i.e. vacation).\n",
    "  Use a holday.<name>.start and holiday.<name>.end to configure a multi-day holiday:\n"
    "  \n"
    "      holiday.sysadmin.name=System Administrator Appreciation Week\n"
    "      holiday.sysadmin.start=20100730\n"
    "      holiday.sysadmin.end=20100805",
    "",
    ""
  );
  items.push_back(multi_holidays);

  /////////////////////////////////////////////////////////////////////////////
  // - Unicode 12

  NewsItem unicode_12 (
    false,
    "Extended Unicode support (Unicode 12)",
    "",
    "",
    "  The support for Unicode character set was improved to support Unicode 12.\n"
    "  This means better support for various language-specific characters - and emojis!",
    "",
    "  Extended unicode support for language specific characters helps non-English users.\n"
    "  While most users don't enter emojis as task metadata, automated task creation tools,\n"
    "  such as bugwarrior, might create tasks with exotic Unicode data.",
    "  You can try it out - 'task add Prepare for an 👽 invasion!'"
  );
  items.push_back(unicode_12);

  /////////////////////////////////////////////////////////////////////////////
  // - The .by attribute modifier

  NewsItem by_modifier (
    false,
    "The .by attribute modifier",
    "",
    "",
    "  A new attribute modifier '.by' was introduced, equivalent to the operator '<='.\n",
    "  This modifier can be used to list all tasks due by the end of the months,\n"
    "  including the last day of the month, using: 'due.by:eom' query",
    "  There was no convenient way to express '<=' relation using attribute modifiers.\n"
    "  As a workaround, instead of 'due.by:eom' one could use 'due.before:eom+1d',\n"
    "  but that requires a certain amount of mental overhead.",
    ""
  );
  items.push_back(by_modifier);

  /////////////////////////////////////////////////////////////////////////////
  // - Context-specific configuration overrides

  NewsItem context_config (
    false,
    "Context-specific configuration overrides",
    "",
    "",
    "  Any context can now define context-specific configuration overrides\n"
    "  via context.<name>.rc.<setting>=<value>.\n",
    "  This allows the user to customize the behaviour of Taskwarrior in a given context,\n"
    "  for example, to change the default command in the 'work' context to 'overdue':\n"
    "\n"
    "      $ task config context.work.rc.default.command overdue\n"
    "\n"
    "  Another example would be to ensure that while context 'work' is active, tasks get\n"
    "  stored in a ~/.worktasks directory:\n"
    "\n"
    "      $ task config context.work.rc.data.location=~/.worktasks",
    "",
    ""
  );
  items.push_back(context_config);

  /////////////////////////////////////////////////////////////////////////////
  // - XDG config home support

  NewsItem xdg_support (
    true,
    "Support for XDG Base Directory Specification",
    "",
    "  The XDG Base Directory specification provides standard locations to store\n"
    "  application data, configuration, state, and cached data in order to keep $HOME\n"
    "  clutter-free. The locations are usually set to ~/.local/share, ~/.config,\n"
    "  ~/.local/state and ~/.cache respectively.",
    "  If taskrc is not found at '~/.taskrc', Taskwarrior will attempt to find it\n"
    "  at '$XDG_CONFIG_HOME/task/taskrc' (defaults to '~/.config/task/taskrc').",
    "",
    "  This allows users to fully follow XDG Base Directory Spec by moving their taskrc:\n"
    "      $ mkdir $XDG_CONFIG_HOME/task\n"
    "      $ mv ~/.taskrc $XDG_CONFIG_HOME/task/taskrc\n\n"
    "  and further setting:\n"
    "      data.location=$XDG_DATA_HOME/task/\n"
    "      hooks.location=$XDG_CONFIG_HOME/task/hooks/\n\n"
    "  Solutions in the past required symlinks or more cumbersome configuration overrides.",
    "  If you configure your data.location and hooks.location as above, ensure\n"
    "  that the XDG_DATA_HOME and XDG_CONFIG_HOME environment variables are set,\n"
    "  otherwise they're going to expand to empty string. Alternatively you can\n"
    "  hardcode the desired paths on your system."
  );
  items.push_back(xdg_support);

  /////////////////////////////////////////////////////////////////////////////
  // - Update holiday data

  NewsItem holidata_2022 (
    false,
    "Updated holiday data for 2022",
    "",
    "",
    "  Holiday data has been refreshed for 2022 and five more holiday locales\n"
    "  have been added: fr-CA, hu-HU, pt-BR, sk-SK and sv-FI.",
    "",
    "  Refreshing the holiday data is part of every release. The addition of the new\n"
    "  locales allows us to better support users in those particular countries."
  );
  items.push_back(holidata_2022);
}

////////////////////////////////////////////////////////////////////////////////
int CmdNews::execute (std::string& output)
{
  auto words = Context::getContext ().cli2.getWords ();
  auto config = Context::getContext ().config;

  // Supress compiler warning about unused argument
  output = "";

  // TODO: 2.6.0 is the only version with explicit release notes, but in the
  // future we need to only execute yet unread release notes
  std::vector<NewsItem> items;
  std::string version = "2.6.0";
  version2_6_0 (items);

  bool full_summary = false;
  bool major_items = true;

  for (auto word: words)
  {
    if (word == "major") {
        major_items = true;
        break;
    }

    if (word == "minor") {
        major_items = false;
        break;
    }

    if (word == "all") {
        full_summary = true;
        break;
    }
  }

  signal (SIGINT, signal_handler);

  // Remove non-major items if displaying a non-full (abbreviated) summary
  int total_highlights = items.size ();
  if (! full_summary)
    items.erase (
      std::remove_if (items.begin (), items.end (), [&](const NewsItem& n){return n._major != major_items;}),
      items.end ()
    );

  // Print release notes
  Color bold = Color ("bold");
  std::cout << bold.colorize (format (
    "\n"
    "==========================================\n"
    "Taskwarrior {1} {2} Release highlights\n"
    "==========================================\n",
    version,
    (full_summary ? "All" : (major_items ? "Major" : "Minor"))
   ));

  for (unsigned short i=0; i < items.size (); i++) {
    std::cout << format ("\n({1}/{2}) ", i+1, items.size ());
    items[i].render ();
  }

  std::cout << "Thank you for catching up on the new features!\n";
  wait_for_enter ();

  // Display outro
  Datetime now;
  Datetime beginning (2006, 11, 29);
  Duration development_time = Duration (now - beginning);

  Color underline = Color ("underline");

  std::stringstream outro;
  outro << underline.colorize (bold.colorize ("Taskwarrior crowdfunding\n"));
  outro << format (
    "Taskwarrior has been in development for {1} years but its survival\n"
    "depends on your support!\n\n"
    "Please consider joining our {2} fundraiser to help us fund maintenance\n"
    "and development of new features:\n\n",
    std::lround (static_cast<float>(development_time.days ()) / 365.25),
    now.year ()
  );
  outro << bold.colorize("    https://github.com/sponsors/GothenburgBitFactory/\n\n");
  outro << "Perks are available for our sponsors.\n";

  std::cout << outro.str ();

  // Set a mark in the config to remember which version's release notes were displayed
  if (config.get ("news.version") != "2.6.0")
  {
    CmdConfig::setConfigVariable ("news.version", "2.6.0", false);

    // Revert back to default signal handling after displaying the outro
    signal (SIGINT, SIG_DFL);

    std::string question = format (
      "\nWould you like to open Taskwarrior {1} fundraising campaign to read more?",
      now.year ()
    );

    std::vector <std::string> options {"yes", "no"};
    std::vector <std::string> matches;

    std::cout << question << " (YES/no) ";

    std::string answer;
    std::getline (std::cin, answer);

    if (std::cin.eof () || trim (answer).empty ())
      answer = "yes";
    else
      lowerCase (trim (answer));

    autoComplete (answer, options, matches, 1); // Hard-coded 1.

    if (matches.size () == 1 && matches[0] == "yes")
#if defined (DARWIN)
      system ("open 'https://github.com/sponsors/GothenburgBitFactory/'");
#else
      system ("xdg-open 'https://github.com/sponsors/GothenburgBitFactory/'");
#endif

    std::cout << std::endl;
  }
  else
    wait_for_enter ();  // Do not display the outro and footnote at once

  if (! full_summary && major_items)
    Context::getContext ().footnote (format (
      "Only major highlights were displayed ({1} out of {2} total).\n"
      "If you're interested in more release highlights, run 'task news {3} minor'.",
      items.size (),
      total_highlights,
      version
    ));

  return 0;
}
