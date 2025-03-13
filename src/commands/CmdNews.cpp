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
// cmake.h include header must come first

#include <CmdNews.h>
#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <Table.h>
#include <format.h>
#include <shared.h>
#include <util.h>

#include <cmath>
#include <csignal>
#include <iostream>

/* Adding a new version:
 *
 * - Add a new `versionX_Y_Z` method to `NewsItem`, and add news items for the new
 *   release.
 * - Call the new method in `NewsItem.all()`. Calls should be in version order.
 * - Test with `task news`.
 */

////////////////////////////////////////////////////////////////////////////////
CmdNews::CmdNews() {
  _keyword = "news";
  _usage = "task          news";
  _description = "Displays news about the recent releases";
  _read_only = true;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::misc;
}

////////////////////////////////////////////////////////////////////////////////
static void signal_handler(int s) {
  if (s == SIGINT) {
    Color footnote;
    if (Context::getContext().color()) {
      if (Context::getContext().config.has("color.footnote"))
        footnote = Color(Context::getContext().config.get("color.footnote"));
    }

    std::cout << "\n\nCome back and read about new features later!\n";

    std::cout << footnote.colorize(
        "\nIf you enjoy Taskwarrior, please consider supporting the project at:\n"
        "    https://github.com/sponsors/GothenburgBitFactory/\n");
    exit(1);
  }
}

void wait_for_enter() {
  signal(SIGINT, signal_handler);

  std::string dummy;
  std::cout << "\nPress enter to continue..";
  std::getline(std::cin, dummy);
  std::cout << "\33[2K\033[A\33[2K";  // Erase current line, move up, and erase again

  signal(SIGINT, SIG_DFL);
}

////////////////////////////////////////////////////////////////////////////////
// Holds information about single improvement / bug.
//
NewsItem::NewsItem(Version version, const std::string& title, const std::string& bg_title,
                   const std::string& background, const std::string& punchline,
                   const std::string& update, const std::string& reasoning,
                   const std::string& actions) {
  _version = version;
  _title = title;
  _bg_title = bg_title;
  _background = background;
  _punchline = punchline;
  _update = update;
  _reasoning = reasoning;
  _actions = actions;
}

void NewsItem::render() {
  auto config = Context::getContext().config;
  Color header;
  Color footnote;
  Color bold;
  Color underline;
  if (Context::getContext().color()) {
    bold = Color("bold");
    underline = Color("underline");
    if (config.has("color.header")) header = Color(config.get("color.header"));
    if (config.has("color.footnote")) footnote = Color(config.get("color.footnote"));
  }

  // TODO: For some reason, bold cannot be blended in 256-color terminals
  // Apply this workaround of colorizing twice.
  std::cout << bold.colorize(header.colorize(format("{1} ({2})\n", _title, _version)));
  if (_background.size()) {
    if (_bg_title.empty()) _bg_title = "Background";

    std::cout << "\n  " << underline.colorize(_bg_title) << std::endl << _background << std::endl;
  }

  wait_for_enter();

  std::cout << "  " << underline.colorize(format("What changed in {1}?\n", _version));
  if (_punchline.size()) std::cout << footnote.colorize(format("{1}\n", _punchline));

  if (_update.size()) std::cout << format("{1}\n", _update);

  wait_for_enter();

  if (_reasoning.size()) {
    std::cout << "  " << underline.colorize("What was the motivation behind this feature?\n")
              << _reasoning << std::endl;
    wait_for_enter();
  }

  if (_actions.size()) {
    std::cout << "  " << underline.colorize("What do I have to do?\n") << _actions << std::endl;
    wait_for_enter();
  }
}

std::vector<NewsItem> NewsItem::all() {
  std::vector<NewsItem> items;
  version2_6_0(items);
  version3_0_0(items);
  version3_1_0(items);
  version3_2_0(items);
  version3_3_0(items);
  version3_4_0(items);
  return items;
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
void NewsItem::version2_6_0(std::vector<NewsItem>& items) {
  Version version("2.6.0");
  /////////////////////////////////////////////////////////////////////////////
  // - Writeable context

  // Detect whether user uses any contexts
  auto config = Context::getContext().config;
  std::stringstream advice;

  auto defined = CmdContext::getContexts();
  if (defined.size()) {
    // Detect the old-style contexts
    std::vector<std::string> old_style;
    std::copy_if(defined.begin(), defined.end(), std::back_inserter(old_style),
                 [&](auto& name) { return config.has("context." + name); });

    if (old_style.size()) {
      advice << format("  You have {1} defined contexts, out of which {2} are old-style:\n",
                       defined.size(),
                       std::count_if(defined.begin(), defined.end(),
                                     [&](auto& name) { return config.has("context." + name); }));

      for (auto context : defined) {
        std::string old_definition = config.get("context." + context);
        if (old_definition != "") advice << format("  * {1}: {2}\n", context, old_definition);
      }

      advice << "\n"
                "  These need to be migrated to new-style, which uses context.<name>.read and\n"
                "  context.<name>.write config variables. Please run the following commands:\n";

      for (auto context : defined) {
        std::string old_definition = config.get("context." + context);
        if (old_definition != "")
          advice << format("  $ task context define {1} '{2}'\n", context, old_definition);
      }

      advice
          << "\n"
             "  Please check these filters are also valid modifications. If a context filter is "
             "not\n"
             "  a valid modification, you can set the context.<name>.write configuration variable "
             "to\n"
             "  specify the write context explicitly. Read more in CONTEXT section of man taskrc.";
    } else
      advice << "  You don't have any old-style contexts defined, so you're good to go as is!";
  } else
    advice << "  You don't have any contexts defined, so you're good to go as is!\n"
              "  Read more about how to use contexts in CONTEXT section of 'man task'.";

  NewsItem writeable_context(
      version, "'Writeable' context", "Background - what is context?",
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
      "  Consider following example, using context 'work' defined as 'project:Work' above:\n"
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
      advice.str());
  items.push_back(writeable_context);

  /////////////////////////////////////////////////////////////////////////////
  // - 64-bit datetime support

  NewsItem uint64_support(
      version, "Support for 64-bit timestamps and numeric values", "", "",
      "  Taskwarrior now supports 64-bit timestamps, making it possible to set due dates\n"
      "  and other date attributes beyond 19 January 2038 (limit of 32-bit timestamps).\n",
      "  The current limit is 31 December 9999 for display reasons (last 4-digit year).",
      "  With each year passing by faster than the last, setting tasks for 2040s\n"
      "  is not as unfeasible as it once was.",
      "  Don't forget that 50-year anniversary and 'task add' a long-term task today!");
  items.push_back(uint64_support);

  /////////////////////////////////////////////////////////////////////////////
  // - Waiting is a virtual status

  NewsItem waiting_status(
      version, "Deprecation of the status:waiting", "",
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
      "  * 'status:pending' should be replaced by 'status:pending -WAITING'");
  items.push_back(waiting_status);

  /////////////////////////////////////////////////////////////////////////////
  // - Support for environment variables in the taskrc

  NewsItem env_vars(
      version, "Environment variables in the taskrc", "", "",
      "  Taskwarrior now supports expanding environment variables in the taskrc file,\n"
      "  allowing users to customize the behaviour of 'task' based on the current env.\n",
      "  The environment variables can either be used in paths, or as separate values:\n"
      "    data.location=$XDG_DATA_HOME/task/\n"
      "    default.project=$PROJECT",
      "", "");
  items.push_back(env_vars);

  /////////////////////////////////////////////////////////////////////////////
  // - Reports outside of context

  NewsItem contextless_reports(
      version, "Context-less reports", "",
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
      "");
  items.push_back(contextless_reports);

  /////////////////////////////////////////////////////////////////////////////
  // - Exporting a particular report

  NewsItem exportable_reports(
      version, "Exporting a particular report", "", "",
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
      "");
  items.push_back(exportable_reports);

  /////////////////////////////////////////////////////////////////////////////
  // - Multi-day holidays

  NewsItem multi_holidays(
      version, "Multi-day holidays", "",
      "  Holidays are currently used in 'task calendar' to visualize the workload during\n"
      "  the upcoming weeks/months. Up to date country-specific holiday data files can be\n"
      "  obtained from our website, holidata.net.",
      "  Instead of single-day holiday entries only, Taskwarrior now supports holidays\n"
      "  that span a range of days (i.e. vacation).\n",
      "  Use a holiday.<name>.start and holiday.<name>.end to configure a multi-day holiday:\n"
      "  \n"
      "      holiday.sysadmin.name=System Administrator Appreciation Week\n"
      "      holiday.sysadmin.start=20100730\n"
      "      holiday.sysadmin.end=20100805",
      "", "");
  items.push_back(multi_holidays);

  /////////////////////////////////////////////////////////////////////////////
  // - Unicode 12

  NewsItem unicode_12(
      version, "Extended Unicode support (Unicode 12)", "", "",
      "  The support for Unicode character set was improved to support Unicode 12.\n"
      "  This means better support for various language-specific characters - and emojis!",
      "",
      "  Extended unicode support for language-specific characters helps non-English speakers.\n"
      "  While most users don't enter emojis as task metadata, automated task creation tools,\n"
      "  such as bugwarrior, might create tasks with exotic Unicode data.",
      "  You can try it out - 'task add Prepare for an 👽 invasion!'");
  items.push_back(unicode_12);

  /////////////////////////////////////////////////////////////////////////////
  // - The .by attribute modifier

  NewsItem by_modifier(
      version, "The .by attribute modifier", "", "",
      "  A new attribute modifier '.by' was introduced, equivalent to the operator '<='.\n",
      "  This modifier can be used to list all tasks due by the end of the months,\n"
      "  including the last day of the month, using: 'due.by:eom' query",
      "  There was no convenient way to express '<=' relation using attribute modifiers.\n"
      "  As a workaround, instead of 'due.by:eom' one could use 'due.before:eom+1d',\n"
      "  but that requires a certain amount of mental overhead.",
      "");
  items.push_back(by_modifier);

  /////////////////////////////////////////////////////////////////////////////
  // - Context-specific configuration overrides

  NewsItem context_config(
      version, "Context-specific configuration overrides", "", "",
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
      "", "");
  items.push_back(context_config);

  /////////////////////////////////////////////////////////////////////////////
  // - XDG config home support

  NewsItem xdg_support(
      version, "Support for XDG Base Directory Specification", "",
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
      "  hardcode the desired paths on your system.");
  items.push_back(xdg_support);

  /////////////////////////////////////////////////////////////////////////////
  // - Update holiday data

  NewsItem holidata_2022(
      version, "Updated holiday data for 2022", "", "",
      "  Holiday data has been refreshed for 2022 and five more holiday locales\n"
      "  have been added: fr-CA, hu-HU, pt-BR, sk-SK and sv-FI.",
      "",
      "  Refreshing the holiday data is part of every release. The addition of the new\n"
      "  locales allows us to better support users in those particular countries.");
  items.push_back(holidata_2022);
}

void NewsItem::version3_0_0(std::vector<NewsItem>& items) {
  Version version("3.0.0");
  NewsItem sync{
      version,
      /*title=*/"New data model and sync backend",
      /*bg_title=*/"",
      /*background=*/"",
      /*punchline=*/
      "The sync functionality for Taskwarrior has been rewritten entirely, and no longer\n"
      "supports taskserver/taskd. The most robust solution is a cloud-storage backend,\n"
      "although a less-mature taskchampion-sync-server is also available. See `task-sync(5)`\n"
      "For details. As part of this change, the on-disk storage format has also changed.\n",
      /*update=*/
      "This is a breaking upgrade: you must export your task database from 2.x and re-import\n"
      "it into 3.x. Hooks run during task import, so if you have any hooks defined,\n"
      "temporarily disable them for this operation.\n\n"
      "See https://taskwarrior.org/docs/upgrade-3/ for information on upgrading to Taskwarrior "
      "3.0.",
  };
  items.push_back(sync);
}

void NewsItem::version3_1_0(std::vector<NewsItem>& items) {
  Version version("3.1.0");
  NewsItem purge{
      version,
      /*title=*/"Purging Tasks, Manually or Automatically",
      /*bg_title=*/"",
      /*background=*/"",
      /*punchline=*/
      "Support for `task purge` has been restored, and new support added for automatically\n"
      "expiring old tasks.\n\n",
      /*update=*/
      "The `task purge` command removes tasks entirely, in contrast to `task delete` which merely\n"
      "sets the task status to 'Deleted'. This functionality existed in versions 2.x but was\n"
      "temporarily removed in 3.0.\n\n"
      "The new `purge.on-sync` configuration parameter controls automatic purging of old tasks.\n"
      "An old task is one with status 'Deleted' that has not been modified in 180 days. This\n"
      "functionality is optional and not enabled by default."};
  items.push_back(purge);
  NewsItem news{
      version,
      /*title=*/"Improved 'task news'",
      /*bg_title=*/"",
      /*background=*/"",
      /*punchline=*/
      "The news you are reading now is improved.\n\n",
      /*update=*/
      "The `task news` command now always shows all new information, not just 'major' news,\n"
      "and will only show that news once. New installs will assume all news has been read.\n"
      "Finally, news can be completely hidden by removing 'news' from the 'verbose' config."};
  items.push_back(news);
}

void NewsItem::version3_2_0(std::vector<NewsItem>& items) {
  Version version("3.2.0");
  NewsItem info{
      version,
      /*title=*/"`task info` Journal Restored",
      /*bg_title=*/"",
      /*background=*/"",
      /*punchline=*/"",
      /*update=*/
      "Support for the \"journal\" output in `task info` has been restored. The command now\n"
      "displays a list of changes made to the task, with timestamps.\n\n"};
  items.push_back(info);
}

void NewsItem::version3_3_0(std::vector<NewsItem>& items) {
  Version version("3.3.0");
  NewsItem info{
      version,
      /*title=*/"AWS S3 Sync",
      /*bg_title=*/"",
      /*background=*/"",
      /*punchline=*/"Use an AWS S3 bucket to sync Taskwarrior",
      /*update=*/
      "Taskwarrior now supports AWS as a backend for sync, in addition to existing support\n"
      "for GCP and taskchampion-sync-server. See `man task-sync` for details.\n\n"};
  items.push_back(info);
}

void NewsItem::version3_4_0(std::vector<NewsItem>& items) {
  Version version("3.4.0");
  NewsItem info{version,
                /*title=*/"Read-Only Access",
                /*bg_title=*/"",
                /*background=*/"",
                /*punchline=*/"Some Taskwarrior commands operate faster in read-only mode",
                /*update=*/
                "Some commands do not need to write to the DB, so can open it in read-only\n"
                "mode and thus more quickly. This does not include reports (task lists),\n"
                "unless the `gc` config is false. Use `rc.gc=0` in command-lines to allow\n"
                "read-only access.\n\n"};
  items.push_back(info);
}

////////////////////////////////////////////////////////////////////////////////
int CmdNews::execute(std::string& output) {
  auto words = Context::getContext().cli2.getWords();
  auto config = Context::getContext().config;

  // Supress compiler warning about unused argument
  output = "";

  std::vector<NewsItem> items = NewsItem::all();
  Version news_version(Context::getContext().config.get("news.version"));
  Version current_version = Version::Current();

  // 2.6.0 is the earliest version with news support.
  if (!news_version.is_valid()) news_version = Version("2.6.0");

  signal(SIGINT, signal_handler);

  // Remove items that have already been shown
  items.erase(std::remove_if(items.begin(), items.end(),
                             [&](const NewsItem& n) { return n._version <= news_version; }),
              items.end());

  Color bold = Color("bold");
  if (items.empty()) {
    std::cout << bold.colorize("You are up to date!\n");
  } else {
    // Print release notes
    std::cout << bold.colorize(
        format("\n"
               "================================================\n"
               "Taskwarrior {1} through {2} Release Highlights\n"
               "================================================\n",
               news_version, current_version));

    for (unsigned short i = 0; i < items.size(); i++) {
      std::cout << format("\n({1}/{2}) ", i + 1, items.size());
      items[i].render();
    }
    std::cout << "Thank you for catching up on the new features!\n";
  }
  wait_for_enter();

  // Set a mark in the config to remember which version's release notes were displayed
  if (news_version < current_version) {
    CmdConfig::setConfigVariable("news.version", std::string(current_version), false);
  }

  return 0;
}

bool CmdNews::should_nag() {
  if (!Context::getContext().verbose("news")) {
    return false;
  }

  Version news_version(Context::getContext().config.get("news.version"));
  if (!news_version.is_valid()) news_version = Version("2.6.0");

  Version current_version = Version::Current();

  if (news_version >= current_version) {
    return false;
  }

  // Check if there are actually any interesting news items to show.
  std::vector<NewsItem> items = NewsItem::all();
  for (auto& item : items) {
    if (item._version > news_version) {
      return true;
    }
  }

  return false;
}
