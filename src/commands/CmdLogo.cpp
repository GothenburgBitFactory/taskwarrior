////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////

#define L10N                                           // Localization complete.

#include <CmdLogo.h>
#include <Context.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdLogo::CmdLogo ()
{
  _keyword     = "logo";
  _usage       = "task logo";
  _description = STRING_CMD_LOGO_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
// Algorithm:
//   Copy file rc.data.location/extensions
//   Generate UUID
//   Call the "install" function once, store results in rc:
//     extension.<uuid>=<JSON>
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

  if (!context.color ())
    throw std::string (STRING_CMD_LOGO_COLOR_REQ);

  std::string indent (context.config.getInteger ("indent.report"), ' ');
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
        sprintf (block, "\033[48;5;%dm  \033[0m", value);
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
        sprintf (block, "\033[48;5;%dm  \033[0m", value);
        output += block;
      }
    }

    output += "\n";
  }

  output += optionalBlankLine ();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
