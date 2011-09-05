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
  _usage       = "task import <file> [<file> ...]";
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
    throw std::string ("You must specify a file to import.");

  std::vector <std::string>::iterator word;
  for (word = words.begin (); word != words.end (); ++word)
  {
    std::string file = *word;
    std::cout << "Importing '" << file << "'\n";

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

            else
              throw std::string ("Unrecognized attribute '") + i->first + "'";
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
        throw std::string ("Not a JSON object: ") + *line;

      delete root;
    }
  }

  context.tdb2.commit ();

  context.footnote (format (STRING_CMD_IMPORT_SUMMARY, count));
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
