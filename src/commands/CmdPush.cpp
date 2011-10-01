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

#include <fstream>
#include <sstream>
#include <Context.h>
#include <Uri.h>
#include <Transport.h>
#include <i18n.h>
#include <text.h>
#include <CmdPush.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdPush::CmdPush ()
{
  _keyword     = "push";
  _usage       = "task          push URL";
  _description = STRING_CMD_PUSH_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
// Transfers the local data (from rc.location._data) to the remote path.
// Because this is potentially on another machine, no checking can be performed.
int CmdPush::execute (std::string& output)
{
  std::vector <std::string> words = context.a3.extract_words ();
  std::string file;
  if (words.size ())
    file = words[0];

  Uri uri (file, "push");
  uri.parse ();

  if (uri._data.length ())
  {
		Directory location (context.config.get ("data.location"));

		Transport* transport;
		if ((transport = Transport::getTransport (uri)) != NULL )
		{
			transport->send (location._data + "/{pending,undo,completed}.data");
			delete transport;
		}
		else
		{
      // Verify that files are not being copied from rc.data.location to the
      // same place.
      if (Directory (uri._path) == Directory (context.config.get ("data.location")))
        throw std::string (STRING_CMD_PUSH_SAME);

      // copy files locally
      if (! Path (uri._data).is_directory ())
        throw format (STRING_CMD_PUSH_NONLOCAL, uri._path);

      std::ifstream ifile1 ((location._data + "/undo.data").c_str(), std::ios_base::binary);
      std::ofstream ofile1 ((uri._path       + "/undo.data").c_str(), std::ios_base::binary);
      ofile1 << ifile1.rdbuf();

      std::ifstream ifile2 ((location._data + "/pending.data").c_str(), std::ios_base::binary);
      std::ofstream ofile2 ((uri._path       + "/pending.data").c_str(), std::ios_base::binary);
      ofile2 << ifile2.rdbuf();

      std::ifstream ifile3 ((location._data + "/completed.data").c_str(), std::ios_base::binary);
      std::ofstream ofile3 ((uri._path       + "/completed.data").c_str(), std::ios_base::binary);
      ofile3 << ifile3.rdbuf();
		}

    output += format (STRING_CMD_PUSH_TRANSFERRED, uri._data) + "\n";
  }
  else
    throw std::string (STRING_CMD_PUSH_NO_URI);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
