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

#include <fstream>
#include <sstream>
#include <Context.h>
#include <Uri.h>
#include <Transport.h>
#include <text.h>
#include <CmdPull.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdPull::CmdPull ()
{
  _keyword     = "pull";
  _usage       = "task pull URL";
  _description = "Overwrites the local *.data files with those found at the URL.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdPull::execute (const std::string&, std::string& output)
{
  std::string file = trim (context.task.get ("description"));

  Uri uri (file, "pull");
  uri.parse ();

  if (uri.data.length ())
  {
		Directory location (context.config.get ("data.location"));

    if (! uri.append ("{pending,undo,completed}.data"))
      throw std::string ("The uri '") + uri.path + "' is not a directory. Did you forget a trailing '/'?";

		Transport* transport;
		if ((transport = Transport::getTransport (uri)) != NULL)
		{
			transport->recv (location.data + "/");
			delete transport;
		}
		else
		{
      // Verify that files are not being copied from rc.data.location to the
      // same place.
      if (Directory (uri.path) == Directory (context.config.get ("data.location")))
        throw std::string ("Cannot pull files when the source and destination are the same.");

      // copy files locally

      // remove {pending,undo,completed}.data
      uri.path = uri.parent();

      Path path1 (uri.path + "undo.data");
      Path path2 (uri.path + "pending.data");
      Path path3 (uri.path + "completed.data");

      if (path1.exists() && path2.exists() && path3.exists())
      {
//        if (confirm ("xxxxxxxxxxxxx"))
//        {
          std::ofstream ofile1 ((location.data + "/undo.data").c_str(), std::ios_base::binary);
          std::ifstream ifile1 (path1.data.c_str()                    , std::ios_base::binary);
          ofile1 << ifile1.rdbuf();

          std::ofstream ofile2 ((location.data + "/pending.data").c_str(), std::ios_base::binary);
          std::ifstream ifile2 (path2.data.c_str()                    , std::ios_base::binary);
          ofile2 << ifile2.rdbuf();

          std::ofstream ofile3 ((location.data + "/completed.data").c_str(), std::ios_base::binary);
          std::ifstream ifile3 (path3.data.c_str()                    , std::ios_base::binary);
          ofile3 << ifile3.rdbuf();
//        }
      }
      else
      {
        throw std::string ("At least one of the database files in '" + uri.path + "' is not present.");
      }
		}

    output += "Tasks transferred from " + uri.data + "\n";
  }
  else
    throw std::string ("No uri was specified for the pull.  Either specify "
                       "the uri of a remote .task directory, or create a "
                       "'pull.default.uri' entry in your .taskrc file.");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
