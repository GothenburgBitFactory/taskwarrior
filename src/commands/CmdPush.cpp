////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
  _usage       = "task          push <URL>";
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

    output += format (STRING_CMD_PUSH_TRANSFERRED, uri.ToString ()) + "\n";
  }
  else
    throw std::string (STRING_CMD_PUSH_NO_URI);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
