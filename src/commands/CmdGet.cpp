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
#include <CmdGet.h>
#include <Variant.h>
#include <Context.h>
#include <DOM.h>
#include <main.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdGet::CmdGet ()
{
  _keyword               = "_get";
  _usage                 = "task          _get <DOM> [<DOM> ...]";
  _description           = STRING_CMD_GET_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
// Rely on the Lexer to correctly identify DOM references, then jsut iterate
// over those.
//
// It is an error to specify no DOM references.
// It is not an error for a DOM reference to resolve to a blank value.
int CmdGet::execute (std::string& output)
{
  std::vector <std::string> results;
  for (auto& arg : context.cli2._args)
  {
    switch (arg._lextype)
    {
    case Lexer::Type::dom:
      {
        Task t;
        Variant result;
        if (getDOM (arg.attribute ("raw"), t, result))
          results.push_back ((std::string) result);
        else
          results.push_back ("");
      }
      break;

    // Look for non-refs to complain about.
    case Lexer::Type::word:
    case Lexer::Type::identifier:
      if (! arg.hasTag ("BINARY") &&
          ! arg.hasTag ("CMD"))
        throw format (STRING_CMD_GET_BAD_REF, arg.attribute ("raw"));

    default:
      break;
    }
  }

  if (results.size () == 0)
    throw std::string (STRING_CMD_GET_NO_DOM);

  join (output, " ", results);
  output += "\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
