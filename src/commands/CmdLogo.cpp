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
#include <CmdLogo.h>
#include <Context.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
CmdLogo::CmdLogo ()
{
  _keyword               = "logo";
  _usage                 = "task          logo";
  _description           = "Displays the Taskwarrior logo";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::misc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdLogo::execute (std::string& output)
{
  static const char* data[] =
  {
    ".........ABDEF",
    ".......BDILNMM",
    ".....ACJNLHFEE",
    "....AFMMFDEEEE",
    "...AFOIDEEEEDD",
    "..AFOHDEEEDIOR",
    "..DMIDEEEEOXXX",
    ".BJMDEEEDMXXXX",
    ".ENFEEEEFVXXXX",
    "BILDEEEEJXXXXX",
    "DLHEEEEDLXXXXX",
    "GMFEEEEDKXXXXX",
    "GMEEEEEEGTIKOR",
    "GMEEEEEEETODDD",
    "GMEEEEEEETXRGE",
    "GMEEEEEEFVXXSE",
    "ENFEEEEEHWXXUE",
    "CLHEEEEDLXXXUE",
    "AILDEEEDRXXXUE",
    ".DNFEEEEPXXXUE",
    ".BJMDEEEEPXXUE",
    "..DMIDEEEDLXUE",
    "..AFOHDEEEDHME",
    "...AFOIDEEEEDE",
    "....AFMMFDEEEE",
    ".....ACJNLHFEE",
    ".......BDILNMM",
    ".........ABDEF",
    ""
  };

  if (! Context::getContext ().color ())
    throw std::string ("The logo command requires that color support is enabled.");

  std::string indent (Context::getContext ().config.getInteger ("indent.report"), ' ');
  output += optionalBlankLine ();

  for (int line = 0; data[line][0]; ++line)
  {
    output += indent;

    for (int c = 0; c < 14; ++c)
    {
      int value = (int) data[line][c];
      if (value == '.')
        output += "  ";
      else
      {
        value += 167;
        char block [24];
        snprintf (block, 24, "\033[48;5;%dm  \033[0m", value);
        output += block;
      }
    }

    for (int c = 13; c >= 0; --c)
    {
      int value = data[line][c];
      if (value == '.')
        output += "  ";
      else
      {
        value += 167;
        char block [24];
        snprintf (block, 24, "\033[48;5;%dm  \033[0m", value);
        output += block;
      }
    }

    output += '\n';
  }

  output += optionalBlankLine ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
