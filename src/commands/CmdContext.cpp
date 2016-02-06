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
#include <CmdContext.h>
#include <CmdConfig.h>
#include <Context.h>
#include <Filter.h>
#include <sstream>
#include <algorithm>
#include <i18n.h>
#include <util.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdContext::CmdContext ()
{
  _keyword               = "context";
  _usage                 = "task          context [<name> | <subcommand>]";
  _description           = STRING_CMD_CONTEXT_USAGE;
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
  auto words = context.cli2.getWords ();
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
    listContexts (out);

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
      value += " ";

    value += words[i];
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
// Returns all user defined contexts.
//
std::vector <std::string> CmdContext::getContexts ()
{
  std::vector <std::string> contexts;

  for (auto& name : context.config)
    if (name.first.substr (0, 8) == "context.")
      contexts.push_back (name.first.substr (8));

  return contexts;
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
  auto confirmation = context.config.getBoolean ("confirmation");

  if (words.size () > 2)
  {
    auto name = "context." + words[1];
    auto value = joinWords (words, 2);

    // Check if the value is a proper filter by filtering current pending.data
    Filter filter;
    std::vector <Task> filtered;
    auto pending = context.tdb2.pending.get_tasks ();

    try
    {
      // This result is not used, and is just to check validity.
      context.cli2.addFilter (value);
      filter.subset (pending, filtered);
    }
    catch (std::string exception)
    {
      throw format (STRING_CMD_CONTEXT_DEF_ABRT2, exception);
    }

    // Make user explicitly confirm filters that are matching no pending tasks
    if (filtered.size () == 0)
      if (confirmation &&
          ! confirm (format (STRING_CMD_CONTEXT_DEF_CONF, value)))
        throw std::string (STRING_CMD_CONTEXT_DEF_ABRT);

    // Set context definition config variable
    bool success = CmdConfig::setConfigVariable (name, value, confirmation);

    if (!success)
      throw format (STRING_CMD_CONTEXT_DEF_FAIL, words[1]);

    out << format (STRING_CMD_CONTEXT_DEF_SUCC, words[1]) << "\n";
  }
  else
    throw std::string (STRING_CMD_CONTEXT_DEF_USAG);
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

    auto confirmation = context.config.getBoolean ("confirmation");
    auto rc = CmdConfig::unsetConfigVariable(name, confirmation);

    // If the currently set context was deleted, unset it
    if (context.config.get ("context") == words[1])
      CmdConfig::unsetConfigVariable("context", false);

    // Output feedback
    if (rc != 0)
      throw format (STRING_CMD_CONTEXT_DEL_FAIL, words[1]);

    out << format (STRING_CMD_CONTEXT_DEL_SUCC, words[1]) << "\n";
  }
  else
    throw std::string(STRING_CMD_CONTEXT_DEL_USAG);
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

    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string", "Name"));
    view.add (Column::factory ("string", "Definition"));
    view.add (Column::factory ("string", "Active"));

    if (context.color ())
    {
      Color label (context.config.get ("color.label"));
      view.colorHeader (label);
    }

    std::string activeContext = context.config.get ("context");

    for (auto& userContext : contexts)
    {
      std::string active = "no";
      if (userContext == activeContext)
          active = "yes";

      int row = view.addRow ();
      view.set (row, 0, userContext);
      view.set (row, 1, context.config.get ("context." + userContext));
      view.set (row, 2, active);
    }

    out << optionalBlankLine ()
        << view.render ()
        << optionalBlankLine ();
  }
  else
    throw std::string (STRING_CMD_CONTEXT_LIST_EMPT);
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
    throw format (STRING_CMD_CONTEXT_SET_NFOU, value);

  // Set the active context.
  // Should always succeed, as we do not require confirmation.
  bool success = CmdConfig::setConfigVariable ("context", value, false);

  if (! success)
    throw format (STRING_CMD_CONTEXT_SET_FAIL, value);

  out << format (STRING_CMD_CONTEXT_SET_SUCC, value) << "\n";
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
  auto currentContext = context.config.get ("context");

  if (currentContext == "")
    out << STRING_CMD_CONTEXT_SHOW_EMPT << "\n";
  else
  {
    std::string currentFilter = context.config.get ("context." + currentContext);
    out << format (STRING_CMD_CONTEXT_SHOW, currentContext, currentFilter) << "\n";
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
    throw std::string(STRING_CMD_CONTEXT_NON_FAIL);

  out << STRING_CMD_CONTEXT_NON_SUCC << "\n";
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionContext::CmdCompletionContext ()
{
  _keyword               = "_context";
  _usage                 = "task          _context";
  _description           = STRING_CMD_HCONTEXT_USAGE;
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
  for (auto& contet : CmdContext::getContexts ())
    output += contet + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
