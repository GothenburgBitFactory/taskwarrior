////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <iostream> // TODO Remove
#include <vector>
#include <Alias.h>
#include <Context.h>
#include <Tree.h>
#include <text.h>

extern Context context;

// Alias expansion limit.  Any more indicates some kind of error.
const int safetyValveDefault = 10;

////////////////////////////////////////////////////////////////////////////////
Alias::Alias ()
{
}

////////////////////////////////////////////////////////////////////////////////
Alias::~Alias ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Alias::load ()
{
  _aliases.clear ();

  std::vector <std::string> vars;
  context.config.all (vars);

  std::vector <std::string>::iterator var;
  for (var = vars.begin (); var != vars.end (); ++var)
    if (var->substr (0, 6) == "alias.")
      _aliases[var->substr (6)] = context.config.get (*var);
}

////////////////////////////////////////////////////////////////////////////////
// An alias must be a distinct word on the command line.
void Alias::resolve (Tree* tree)
{
  bool something;
  int safety_valve = safetyValveDefault;

  do
  {
    something = false;

    std::string command;
    std::vector <Tree*>::iterator i;
    for (i = tree->_branches.begin (); i != tree->_branches.end (); ++i)
    {
      // Parser override operator.
      if ((*i)->attribute ("raw") == "--")
        break;

      // Skip known args.
      if (! (*i)->hasTag ("?"))
        continue;

      std::string raw = (*i)->attribute ("raw");
      std::map <std::string, std::string>::iterator match = context.aliases.find (raw);
      if (match != context.aliases.end ())
      {
        something = true;

        std::vector <std::string> words;
        splitq (words, context.aliases[raw], ' ');

        std::vector <std::string>::iterator word;
        for (word = words.begin (); word != words.end (); ++word)
        {
          // TODO Insert branch (words) in place of (*i).
          std::cout << "# alias word '" << *word << "'\n";
        }
      }
    }
  }
  while (something && --safety_valve > 0);

  if (safety_valve <= 0)
    context.debug (format ("Nested alias limit of {1} reached.", safetyValveDefault));
}

////////////////////////////////////////////////////////////////////////////////
