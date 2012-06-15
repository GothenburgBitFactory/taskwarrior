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
#include <util.h>
#include <CmdMerge.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdMerge::CmdMerge ()
{
  _keyword     = "merge";
  _usage       = "task          merge <URL>";
  _description = STRING_CMD_MERGE_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdMerge::execute (std::string& output)
{
  // invoke gc and commit before merging in order to update data files
  context.tdb2.gc ();
  context.tdb2.commit ();

  std::vector <std::string> words = context.a3.extract_words ();
  std::string file;
  if (words.size ())
    file = words[0];

  std::string pushfile = "";
  std::string tmpfile = "";

  std::string sAutopush = lowerCase (context.config.get        ("merge.autopush"));
  bool        bAutopush =            context.config.getBoolean ("merge.autopush");

  Uri uri (file, "merge");
  uri.parse();

  if (uri._data.length ())
  {
    Directory location (context.config.get ("data.location"));

    // be sure that uri points to a file
    uri.append ("undo.data");

    Transport* transport;
    if ((transport = Transport::getTransport (uri)) != NULL )
    {
      tmpfile = location._data + "/undo_remote.data";
      transport->recv (tmpfile);
      delete transport;

      file = tmpfile;
    }
    else
      file = uri._path;

    // XXX the following function could indicate whether a modification was
    // performed without an exception (by returning a boolean, within a status
    // object or with a specific function)
    try
    {
        context.tdb2.merge (file);
    }
    catch (std::string& e) {
        if (e == STRING_TDB2_UP_TO_DATE)
        {
            output += e + "\n";
            return 0;
        }
        else
            throw e;
    }

    output += std::string (STRING_CMD_MERGE_COMPLETE) + "\n";

    if (tmpfile != "")
      remove (tmpfile.c_str ());

    if (((sAutopush == "ask") && (confirm (format (STRING_CMD_MERGE_CONFIRM, uri.ToString ()))))
       || (bAutopush))
    {
      // Derive autopush uri from merge.default.uri? otherwise: change prompt above

      // Change the "merge" command to "push".
      std::vector <Arg>::iterator i;
      for (i = context.a3.begin (); i != context.a3.end (); ++i)
      {
        if (i->_category == Arg::cat_command)
        {
          i->_raw = "push";
          break;
        }
      }

      // Append the URI argument.
      context.a3.push_back (Arg (uri._data, Arg::cat_literal));

      std::string out;
      context.commands["push"]->execute (out);
    }
  }
  else
    throw std::string (STRING_CMD_MERGE_NO_URI);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
