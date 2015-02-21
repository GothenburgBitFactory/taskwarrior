////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <Context.h>
#include <sstream>
#include <algorithm>
#include <i18n.h>
#include <text.h>
#include <CmdContext.h>
#include <CmdConfig.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdContext::CmdContext ()
{
  _keyword     = "context";
  _usage       = "task          context [name [value | '']]";
  _description = STRING_CMD_CONTEXT_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdContext::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  // Get the non-attribute, non-fancy command line arguments.
  std::vector <std::string> words = context.cli.getWords ();

  if (words.size () > 0)
  {
    std::string subcommand = words[0];

    if (subcommand == "define")
      rc = defineContext(words, out);
    else if (subcommand == "delete")
      rc = deleteContext(words, out);
    else if (subcommand == "list")
      rc = listContexts(words, out);
    else
      rc = setContext(words, out);
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
// Joins all the words in the specified interval <from, to) to a one string,
// which is returned.
//
// If to is specified as 0, all remaining words will be joined.
//
std::string CmdContext::joinWords (std::vector <std::string>& words, unsigned int from, unsigned int to /* = 0 */)
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

  Config::const_iterator name;
  for (name = context.config.begin (); name != context.config.end (); ++name)
  {
    if (name->first.substr (0, 8) == "context.")
    {
      contexts.push_back (name->first.substr (8));
    }
  }

  return contexts;
}

////////////////////////////////////////////////////////////////////////////////
int CmdContext::defineContext (std::vector <std::string>& words, std::stringstream& out)
{
  // task context define home project:Home
  if (words.size () > 2)
  {
    std::string name = "context." + words[1];
    std::string value = joinWords(words, 2);
    // TODO: Check if the value is a proper filter

    bool confirmation = context.config.getBoolean ("confirmation");
    bool success = CmdConfig::setConfigVariable(name, value, confirmation);

    if (success)
      out << "Context '" << words[1] << "' successfully defined." << "\n";
    else
      out << "Context '" << words[1] << "' was not defined." << "\n";
  }
  else
    throw "You have to specify both context name and definition.";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int CmdContext::deleteContext (std::vector <std::string>& words, std::stringstream& out)
{
  // task context delete home
  if (words.size () > 1)
  {
    std::string name = "context." + words[1];

    bool confirmation = context.config.getBoolean ("confirmation");
    int status = CmdConfig::unsetConfigVariable(name, confirmation);

    std::string currentContext = context.config.get ("context");

    if (currentContext == words[1])
      CmdConfig::unsetConfigVariable("context", false);

    if (status == 0)
      out << "Context '" << words[1] << "' successfully undefined." << "\n";
    else
      out << "Context '" << words[1] << "' was not undefined." << "\n";
  }
  else
    throw "You have to specify context name.";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int CmdContext::listContexts (std::vector <std::string>& words, std::stringstream& out)
{
  int rc = 0;
  std::vector <std::string> contexts = getContexts();

  if (contexts.size ())
  {
    std::sort (contexts.begin (), contexts.end ());

    // Render a list of UDA name, type, label, allowed values,
    // possible default value, and finally the usage count.
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string", "Name"));
    view.add (Column::factory ("string", "Definition"));

    Color label (context.config.get ("color.label"));
    view.colorHeader (label);

    std::vector <std::string>::iterator userContext;
    for (userContext = contexts.begin (); userContext != contexts.end (); ++userContext)
    {
      std::string definition = context.config.get ("context." + *userContext);

      int row = view.addRow ();
      view.set (row, 0, *userContext);
      view.set (row, 1, definition);
    }

    out << optionalBlankLine ()
        << view.render ()
        << optionalBlankLine ();
  }
  else
  {
    out << "No contexts defined." << "\n";
    rc = 1;
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdContext::setContext (std::vector <std::string>& words, std::stringstream& out)
{
  // task context home
  std::string value = words[0];
  std::vector <std::string> contexts = getContexts ();

  if (std::find (contexts.begin (), contexts.end (), value) == contexts.end())
    throw format ("Context '{1}' not found.", value);

  bool success = CmdConfig::setConfigVariable("context", value, false);

  if (success)
    out << "Context '" << value << "' applied." << "\n";
  else
    out << "Context '" << value << "' was not applied." << "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionContext::CmdCompletionContext ()
{
  _keyword     = "_context";
  _usage       = "task          _context";
  _description = STRING_CMD_HCONTEXT_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionContext::execute (std::string& output)
{
  std::vector <std::string> userContexts = CmdContext::getContexts ();

  std::vector <std::string>::iterator userContext;
  for (userContext = userContexts.begin (); userContext != userContexts.end (); ++userContext)
    output += *userContext + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
