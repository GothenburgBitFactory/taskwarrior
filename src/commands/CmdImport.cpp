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

#define L10N                                           // Localization complete.

#include <iostream>
#include <sstream>
#include <Context.h>
#include <Transport.h>
#include <JSON.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>
#include <CmdImport.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdImport::CmdImport ()
{
  _keyword     = "import";
  _usage       = "task          import <file> [<file> ...]";
  _description = STRING_CMD_IMPORT_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdImport::execute (std::string& output)
{
  int rc = 0;
  int count = 0;

	// Use the description as a file name.
  std::vector <std::string> words = context.a3.extract_words ();
  if (! words.size ())
    throw std::string (STRING_CMD_IMPORT_NOFILE);

  std::vector <std::string>::iterator word;
  for (word = words.begin (); word != words.end (); ++word)
  {
    std::string file = *word;
    std::cout << format (STRING_CMD_IMPORT_FILE, file) << "\n";

    std::string tmpfile = "";
    Uri uri (file);
    uri.parse ();

    Transport* transport;
    if ((transport = Transport::getTransport (uri)) != NULL)
    {
      std::string location (context.config.get ("data.location"));
      tmpfile = location + "/import.data";
      transport->recv (tmpfile);
      delete transport;

      file = tmpfile;
    }

    // Load the file.
    std::vector <std::string> lines;
    File::read (file, lines);

    std::vector <std::string>::iterator line;
    for (line = lines.begin (); line != lines.end (); ++line)
    {
      std::string object = trimLeft (
                             trimRight (
                               trimRight (
                                 trim (*line), ","),
                               "]"),
                             "[");
      // Skip blanks.  May be caused by the trim calls above.
      if (! object.length ())
        continue;

      // Parse the whole thing.
      json::value* root = json::parse (object);
      if (root->type () == json::j_object)
      {
        json::object* root_obj = (json::object*)root;
        Task task;

        // For each object element...
        json_object_iter i;
        for (i  = root_obj->_data.begin ();
             i != root_obj->_data.end ();
             ++i)
        {
          // If the attribute is a recognized column.
          Column* col = context.columns[i->first];
          if (col)
          {
            // Any specified id is ignored.
            if (i->first == "id")
              ;

            // Urgency, if present, is ignored.
            else if (i->first == "urgency")
              ;

            // Dates are converted from ISO to epoch.
            else if (col->type () == "date")
            {
              Date d (unquoteText (i->second->dump ()));
              task.set (i->first, d.toEpochString ());
            }

            // Tags are an array of JSON strings.
            else if (i->first == "tags")
            {
              json::array* tags = (json::array*)i->second;
              json_array_iter t;
              for (t  = tags->_data.begin ();
                   t != tags->_data.end ();
                   ++t)
              {
                json::string* tag = (json::string*)*t;
                task.addTag (tag->_data);
              }
            }

            // Other types are simply added.
            else
              task.set (i->first, unquoteText (i->second->dump ()));
          }

          // Several attributes do not have columns.
          //   mask
          //   imask
          //   parent
          //   UDA orphans
          else
          {
            // Annotations are an array of JSON objects with 'entry' and
            // 'description' values and must be converted.
            if (i->first == "annotations")
            {
              std::map <std::string, std::string> annos;

              json::array* atts = (json::array*)i->second;
              json_array_iter annotations;
              for (annotations  = atts->_data.begin ();
                   annotations != atts->_data.end ();
                   ++annotations)
              {
                json::object* annotation = (json::object*)*annotations;
                json::string* when = (json::string*)annotation->_data["entry"];
                json::string* what = (json::string*)annotation->_data["description"];

                if (! when)
                  throw format (STRING_CMD_IMPORT_NO_ENTRY, *line);

                if (! what)
                  throw format (STRING_CMD_IMPORT_NO_DESC, *line);

                std::string name = "annotation_" + Date (when->_data).toEpochString ();

                annos.insert (std::make_pair (name, what->_data));
              }

              task.setAnnotations (annos);
            }

            // Attributes without columns are simply added.
            else if (i->first == "parent" ||
                     i->first == "mask"   ||
                     i->first == "imask")
            {
              task.set (i->first, unquoteText (i->second->dump ()));
            }

            // UDA Orphan - must be preserved.
            else
            {
              task.set (i->first, unquoteText (i->second->dump ()));
            }
          }
        }

        context.tdb2.add (task);
        ++count;
        std::cout << "  "
                  << task.get ("uuid")
                  << " "
                  << task.get ("description")
                  << "\n";
      }
      else
        throw format (STRING_CMD_IMPORT_NOT_JSON, *line);

      delete root;
    }
  }

  context.tdb2.commit ();

  context.footnote (format (STRING_CMD_IMPORT_SUMMARY, count));
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
