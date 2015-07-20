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
#include <Variant.h>
#include <Context.h>
#include <main.h>
#include <text.h>
#include <i18n.h>
#include <CmdGet.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdGet::CmdGet ()
{
  _keyword     = "_get";
  _usage       = "task          _get <DOM> [<DOM> ...]";
  _description = STRING_CMD_GET_USAGE;
  _read_only   = true;
  _displays_id = false;
  _category    = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdGet::execute (std::string& output)
{
  // Obtain the arguments from the description.  That way, things like '--'
  // have already been handled.
  std::vector <std::string> words = context.cli2.getWords (false);
  if (words.size () == 0)
    throw std::string (STRING_CMD_GET_NO_DOM);

  bool found = false;
  std::vector <std::string> results;
  for (auto& word : words)
  {
    Task t;
    Variant result;
    if (context.dom.get (word, t, result))
    {
      results.push_back ((std::string) result);
      found = true;
    }
  }

  join (output, " ", results);
  output += "\n";
  return found ? 0 : 1;
}

////////////////////////////////////////////////////////////////////////////////
