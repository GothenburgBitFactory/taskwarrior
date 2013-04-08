////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

#include <fstream>
#include <sstream>
#include <Context.h>
#include <Uri.h>
#include <Transport.h>
#include <i18n.h>
#include <text.h>
#include <CmdPull.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdPull::CmdPull ()
{
  _keyword     = "pull";
  _usage       = "task          pull <URL>";
  _description = STRING_CMD_PULL_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdPull::execute (std::string& output)
{
  std::vector <std::string> words = context.a3.extract_words ();
  std::string file;
  if (words.size ())
    file = words[0];

  Uri uri (file, "pull");
  uri.parse ();

  if (uri._data.length ())
  {
		Directory location (context.config.get ("data.location"));

    if (! uri.append ("{pending,undo,completed}.data"))
      throw format (STRING_CMD_PULL_NOT_DIR, uri._path);

		Transport* transport;
		if ((transport = Transport::getTransport (uri)) != NULL)
		{
			transport->recv (location._data + "/");
			delete transport;
		}
		else
		{
      // Verify that files are not being copied from rc.data.location to the
      // same place.
      if (Directory (uri._path) == Directory (context.config.get ("data.location")))
        throw std::string (STRING_CMD_PULL_SAME);

      // copy files locally

      // remove {pending,undo,completed}.data
      uri._path = uri.parent();

      Path path1 (uri._path + "undo.data");
      Path path2 (uri._path + "pending.data");
      Path path3 (uri._path + "completed.data");

      if (path1.exists() && path2.exists() && path3.exists())
      {
//        if (confirm ("xxxxxxxxxxxxx"))
//        {
          std::ofstream ofile1 ((location._data + "/undo.data").c_str(), std::ios_base::binary);
          std::ifstream ifile1 (path1._data.c_str()                    , std::ios_base::binary);
          ofile1 << ifile1.rdbuf();

          std::ofstream ofile2 ((location._data + "/pending.data").c_str(), std::ios_base::binary);
          std::ifstream ifile2 (path2._data.c_str()                    , std::ios_base::binary);
          ofile2 << ifile2.rdbuf();

          std::ofstream ofile3 ((location._data + "/completed.data").c_str(), std::ios_base::binary);
          std::ifstream ifile3 (path3._data.c_str()                    , std::ios_base::binary);
          ofile3 << ifile3.rdbuf();
//        }
      }
      else
      {
        throw format (STRING_CMD_PULL_MISSING, uri._path);
      }
		}

    output += format (STRING_CMD_PULL_TRANSFERRED, uri.ToString ()) + "\n";
  }
  else
    throw std::string (STRING_CMD_PULL_NO_URI);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
