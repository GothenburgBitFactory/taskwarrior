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
#include <CmdContext.h>
#include <CmdConfig.h>
#include <Table.h>
#include <Context.h>
#include <Filter.h>
#include <sstream>
#include <algorithm>
#include <set>
#include <format.h>
#include <main.h>
#include <shared.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
CmdContext::CmdContext ()
{
  _keyword               = "context";
  _usage                 = "task          context [<name> | <subcommand>]";
  _description           = "Set and define contexts (default filters / modifications)";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::context;
}

////////////////////////////////////////////////////////////////////////////////
int CmdContext::execute (std::string& output)
{
  std::stringstream out;

  // Get the non-attribute, non-fancy command line arguments.
  auto words = Context::getContext ().cli2.getWords ();
  if (words.size () > 0)
  {
    auto subcommand = words[0];

         if (subcommand == "define") defineContext (words, out);
    else if (subcommand == "delete") deleteContext (words, out);
    else if (subcommand == "list")   listContexts (out);
    else if (subcommand == "none")   unsetContext (out);
    else if (subcommand == "show")   showContext (out);
    else if (words.size ())          setContext (words, out);
  }
  else
  {
    listContexts (out);
    out << "Use 'task context none' to unset the current context.\n";
  }

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Joins all the words in the specified interval <from, to) to one string,
// which is then returned.
//
// If to is specified as 0 (default value), all the remaining words will be joined.
//
std::string CmdContext::joinWords (const std::vector <std::string>& words, unsigned int from, unsigned int to /* = 0 */)
{
  std::string value = "";

  if (to == 0)
    to = words.size();

  for (unsigned int i = from; i < to; ++i)
  {
    if (i > from)
      value += ' ';

    value += words[i];
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
// Validate the context as valid for writing and fail the write context definition
// A valid write context:
//   - does not contain any operators except AND
//   - does not contain tag exclusion
//   - does not use modifiers, except for 'equals' and 'is'
//
// Returns True if the context is a valid write context. If the context is
// invalid due to a wrong modifier use, the modifier string will contain the
// first invalid modifier.
//
bool CmdContext::validateWriteContext (const std::vector <A2>& lexedArgs, std::string& reason)
{
  for (auto &arg: lexedArgs) {
    if (arg._lextype == Lexer::Type::op)
      if (arg.attribute ("raw") == "or")
      {
        reason = "contains the 'OR' operator";
        return false;
      }

    if (arg._lextype == Lexer::Type::pair) {
      auto modifier = arg.attribute ("modifier");
      if (modifier != "" && modifier != "is" && modifier != "equals")
      {
        reason = format ("contains an attribute modifier '{1}'", arg.attribute ("raw"));
        return false;
      }
    }

    if (arg._lextype == Lexer::Type::tag) {
      if (arg.attribute ("sign") == "-")
      {
        reason = format ("contains tag exclusion '{1}'", arg.attribute ("raw"));
        return false;
      }
    }

  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Returns all user defined contexts.
//
std::vector <std::string> CmdContext::getContexts ()
{
  std::set <std::string> contexts;

  for (auto& name : Context::getContext ().config)
    if (name.first.substr (0, 8) == "context.")
    {
      std::string suffix = name.first.substr (8);

      if (suffix.find (".") != std::string::npos)
        contexts.insert (suffix.substr (0, suffix.find (".")));
      else
        contexts.insert (suffix);
    }

  return std::vector <std::string> (contexts.begin (), contexts.end ());
}

////////////////////////////////////////////////////////////////////////////////
// Defines a new user-provided context.
//  - The context definition is written into .taskrc as a context.<name> variable.
//  - Deletion of the context requires confirmation if rc.confirmation=yes.
//
// Returns: 0 if the addition of the config variable was successful, 1 otherwise
//
// Invoked with: task context define <name> <filter>
// Example:      task context define home project:Home
//
void CmdContext::defineContext (const std::vector <std::string>& words, std::stringstream& out)
{
  auto config = Context::getContext ().config;
  bool confirmation = config.getBoolean ("confirmation");

  if (words.size () > 2)
  {
    auto name = "context." + words[1];
    auto value = joinWords (words, 2);

    // Make sure nobody creates a context with name 'list', 'none' or 'show'
    if (words[1] == "none" or words[1] == "list" or words[1] == "show")
    {
      throw format ("The name '{1}' is reserved and not allowed to use as a context name.", words[1]);
    }

    // Extract MISCELLANEOUS arguments (containing the filter definition) for later analysis
    std::vector <A2> lexedArgs = Context::getContext ().cli2.getMiscellaneous();

    // Check if the value is a proper filter by filtering current pending.data
    Filter filter;
    std::vector <Task> filtered;
    auto pending = Context::getContext ().tdb2.pending.get_tasks ();

    try
    {
      // This result is not used, and is just to check validity.
      Context::getContext ().cli2.addFilter (value);
      filter.subset (pending, filtered);
    }
    catch (std::string exception)
    {
      throw format ("Filter validation failed: {1}", exception);
    }

    // Make user explicitly confirm filters that are matching no pending tasks
    if (filtered.size () == 0)
      if (confirmation &&
          ! confirm (format ("The filter '{1}' matches 0 pending tasks. Do you wish to continue?", value)))
        throw std::string ("Context definition aborted.");

    std::string reason = "";
    bool valid_write_context = CmdContext::validateWriteContext (lexedArgs, reason);

    if (! valid_write_context)
    {
      std::stringstream warning;
      warning << format ("The filter '{1}' is not a valid modification string, because it contains {2}.", value, reason)
              << "\nAs such, value for the write context cannot be set (context will not apply on task add / task log).\n\n"
              << format ("Please use 'task config context.{1}.write <default mods>' to set default attribute values for new tasks in this context manually.\n\n", words[1]);
      out << colorizeFootnote (warning.str ());
    }

    // Set context definition config variable
    bool read_success = CmdConfig::setConfigVariable (name + ".read", value, confirmation);
    bool write_success = false;

    if (valid_write_context)
      write_success = CmdConfig::setConfigVariable (name + ".write", value, confirmation);

    // Remove old-school context name, if it exists, assuming the read context was defined
    if (read_success)
      if (config.has (name)) {
        CmdConfig::unsetConfigVariable (name, false);
      }

    if (!read_success and !write_success)
      throw format ("Context '{1}' not defined.", words[1]);
    else if (!read_success)
      out << format ("Context '{1}' defined (write only).", words[1]);
    else if (!write_success)
      out << format ("Context '{1}' defined (read only).", words[1]);
    else
      out << format ("Context '{1}' defined (read, write).", words[1]);

    out << format (" Use 'task context {1}' to activate.\n", words[1]);
  }
  else
    throw std::string ("Both context name and its definition must be provided.");
}

////////////////////////////////////////////////////////////////////////////////
// Deletes the specified context.
//  - If the deleted context is currently active, unset it.
//  - Deletion of the context requires confirmation if rc.confirmation=yes.
//
// Returns: 0 if the removal of the config variable was successful, 1 otherwise
//
// Invoked with: task context delete <name>
// Example:      task context delete home
//
void CmdContext::deleteContext (const std::vector <std::string>& words, std::stringstream& out)
{
  if (words.size () > 1)
  {
    // Delete the specified context
    auto name = "context." + words[1];

    auto confirmation = Context::getContext ().config.getBoolean ("confirmation");
    if (confirmation && ! confirm (format ("Do you want to delete context '{1}'?", words[1])))
      throw format ("Context '{1}' not deleted.", words[1]);

    // Delete legacy format and .read / .write flavours
    auto rc = CmdConfig::unsetConfigVariable(name, false);
    rc     += CmdConfig::unsetConfigVariable(name + ".read", false);
    rc     += CmdConfig::unsetConfigVariable(name + ".write", false);

    // If the currently set context was deleted, unset it
    if (Context::getContext ().config.get ("context") == words[1])
      CmdConfig::unsetConfigVariable("context", false);

    // Output feedback, rc should be even because only 0 (found and removed)
    // and 2 (not found) are aceptable return values from unsetConfigVariable
    if (rc % 2 != 0)
      throw format ("Context '{1}' not deleted.", words[1]);
    else if (rc == 6)
      throw format ("Context '{1}' not found.", words[1]);

    out << format ("Context '{1}' deleted.\n", words[1]);
  }
  else
    throw std::string("Context name needs to be specified.");
}

////////////////////////////////////////////////////////////////////////////////
// Render a list of context names and their definitions.
//
// Returns: 0 the resulting list is non-empty, 1 otherwise
//
// Invoked with: task context list
// Example:      task context list
//
void CmdContext::listContexts (std::stringstream& out)
{
  auto contexts = getContexts();
  if (contexts.size ())
  {
    std::sort (contexts.begin (), contexts.end ());

    Table table;
    table.width (Context::getContext ().getWidth ());
    table.add ("Name");
    table.add ("Type");
    table.add ("Definition");
    table.add ("Active");
    setHeaderUnderline (table);

    std::string activeContext = Context::getContext ().config.get ("context");

    for (auto& userContext : contexts)
    {
      std::string active = "no";
      if (userContext == activeContext)
          active = "yes";

      int row = table.addRow ();
      table.set (row, 0, userContext);
      table.set (row, 1, "read");
      table.set (row, 2, Context::getContext ().getTaskContext("read", userContext));
      table.set (row, 3, active);

      row = table.addRow ();
      table.set (row, 0, "");
      table.set (row, 1, "write");
      table.set (row, 2, Context::getContext ().getTaskContext("write", userContext));
      table.set (row, 3, active);
    }

    out << optionalBlankLine ()
        << table.render ()
        << optionalBlankLine ();
  }
  else
    throw std::string ("No contexts defined.");
}

////////////////////////////////////////////////////////////////////////////////
// Sets the specified context as currently active.
//   - If some other context was active, the value of currently active context
//     is replaced, not added.
//   - Setting of the context does not require confirmation.
//
// Returns: 0 if the setting of the context was successful, 1 otherwise
//
// Invoked with: task context <name>
// Example:      task context home
//
void CmdContext::setContext (const std::vector <std::string>& words, std::stringstream& out)
{
  auto value = words[0];
  auto contexts = getContexts ();

  // Check that the specified context is defined
  if (std::find (contexts.begin (), contexts.end (), value) == contexts.end ())
    throw format ("Context '{1}' not found.", value);

  // Set the active context.
  // Should always succeed, as we do not require confirmation.
  bool success = CmdConfig::setConfigVariable ("context", value, false);

  if (! success)
    throw format ("Context '{1}' not applied.", value);

  out << format ("Context '{1}' set. Use 'task context none' to remove.\n", value);
}

////////////////////////////////////////////////////////////////////////////////
// Shows the currently active context.
//
// Returns: Always returns 0.
//
// Invoked with: task context show
// Example:      task context show
//
void CmdContext::showContext (std::stringstream& out)
{
  auto currentContext = Context::getContext ().config.get ("context");

  if (currentContext == "")
    out << "No context is currently applied.\n";
  else
  {
    out << format (
      "Context '{1}' with \n\n* read filter: '{2}'\n* write filter: '{3}'\n\nis currently applied.\n",
      currentContext,
      Context::getContext ().getTaskContext("read", ""),
      Context::getContext ().getTaskContext("write", "")
    );
  }
}

////////////////////////////////////////////////////////////////////////////////
// Unsets the currently active context.
//   - Unsetting of the context does not require confirmation.
//
// Returns: 0 if the unsetting of the context was successful, 1 otherwise (also
//          returned if no context is currently active)
//
// Invoked with: task context none
// Example:      task context none
//
void CmdContext::unsetContext (std::stringstream& out)
{
  if (CmdConfig::unsetConfigVariable ("context", false))
    throw std::string ("Context not unset.");

  out << "Context unset.\n";
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionContext::CmdCompletionContext ()
{
  _keyword               = "_context";
  _usage                 = "task          _context";
  _description           = "Lists all supported contexts, for completion purposes";
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
int CmdCompletionContext::execute (std::string& output)
{
  for (auto& context : CmdContext::getContexts ())
    output += context + '\n';

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
