////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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

#include <sstream>
#include <string>
#include "Nibbler.h"
#include "T2.h"
#include "text.h"
#include "util.h"

////////////////////////////////////////////////////////////////////////////////
T2::T2 ()
{
  // Each new task gets a uuid.
  set ("uuid", uuid ());
}

////////////////////////////////////////////////////////////////////////////////
// Attempt an FF4 parse first, using Record::parse, and in the event of an error
// try a legacy parse (F3, FF2).  Note that FF1 is no longer supported.
T2::T2 (const std::string& input)
{
  try
  {
    parse (input);
  }

  catch (std::string& e)
  {
    legacyParse (input);
  }
}

////////////////////////////////////////////////////////////////////////////////
T2& T2::operator= (const T2& other)
{
  throw std::string ("unimplemented T2::operator=");
  if (this != &other)
  {
    sequence = other.sequence;
    subst    = other.subst;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
T2::~T2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Support FF2, FF3.
void T2::legacyParse (const std::string& line)
{
  switch (determineVersion (line))
  {
  // File format version 1, from 2006.11.27 - 2007.12.31
  case 1:
    throw std::string ("Task no longer supports file format 1, originally used "
                       "between 27 November 2006 and 31 December 2007.");
    break;

  // File format version 2, from 2008.1.1 - 2009.3.23
  case 2:
/*
    {
      if (line.length () > 46)       // ^.{36} . \[\] \[\] \n
      {
        mUUID = line.substr (0, 36);

        mStatus =   line[37] == '+' ? completed
                  : line[37] == 'X' ? deleted
                  : line[37] == 'r' ? recurring
                  :                   pending;

        size_t openTagBracket  = line.find ("[");
        size_t closeTagBracket = line.find ("]", openTagBracket);
        if (openTagBracket  != std::string::npos &&
            closeTagBracket != std::string::npos)
        {
          size_t openAttrBracket  = line.find ("[", closeTagBracket);
          size_t closeAttrBracket = line.find ("]", openAttrBracket);
          if (openAttrBracket  != std::string::npos &&
              closeAttrBracket != std::string::npos)
          {
            std::string tags = line.substr (
              openTagBracket + 1, closeTagBracket - openTagBracket - 1);
            std::vector <std::string> rawTags;
            split (mTags, tags, ' ');

            std::string attributes = line.substr (
              openAttrBracket + 1, closeAttrBracket - openAttrBracket - 1);
            std::vector <std::string> pairs;
            split (pairs, attributes, ' ');
            for (size_t i = 0; i <  pairs.size (); ++i)
            {
              std::vector <std::string> pair;
              split (pair, pairs[i], ':');
              if (pair.size () == 2)
                mAttributes[pair[0]] = pair[1];
            }

            mDescription = line.substr (closeAttrBracket + 2, std::string::npos);
          }
          else
            throw std::string ("Missing attribute brackets");
        }
        else
          throw std::string ("Missing tag brackets");
      }
      else
        throw std::string ("Line too short");

      mAnnotations.clear ();
    }
*/
    break;

  // File format version 3, from 2009.3.23
  case 3:
/*
    {
      if (line.length () > 49)       // ^.{36} . \[\] \[\] \[\] \n
      {
        mUUID = line.substr (0, 36);

        mStatus =   line[37] == '+' ? completed
                  : line[37] == 'X' ? deleted
                  : line[37] == 'r' ? recurring
                  :                   pending;

        size_t openTagBracket  = line.find ("[");
        size_t closeTagBracket = line.find ("]", openTagBracket);
        if (openTagBracket  != std::string::npos &&
            closeTagBracket != std::string::npos)
        {
          size_t openAttrBracket  = line.find ("[", closeTagBracket);
          size_t closeAttrBracket = line.find ("]", openAttrBracket);
          if (openAttrBracket  != std::string::npos &&
              closeAttrBracket != std::string::npos)
          {
            size_t openAnnoBracket  = line.find ("[", closeAttrBracket);
            size_t closeAnnoBracket = line.find ("]", openAnnoBracket);
            if (openAnnoBracket  != std::string::npos &&
                closeAnnoBracket != std::string::npos)
            {
              std::string tags = line.substr (
                openTagBracket + 1, closeTagBracket - openTagBracket - 1);
              std::vector <std::string> rawTags;
              split (mTags, tags, ' ');

              std::string attributes = line.substr (
                openAttrBracket + 1, closeAttrBracket - openAttrBracket - 1);
              std::vector <std::string> pairs;
              split (pairs, attributes, ' ');

              for (size_t i = 0; i <  pairs.size (); ++i)
              {
                std::vector <std::string> pair;
                split (pair, pairs[i], ':');
                if (pair.size () == 2)
                  mAttributes[pair[0]] = pair[1];
              }

              // Extract and split the annotations, which are of the form:
              //   1234:"..." 5678:"..."
              std::string annotations = line.substr (
                openAnnoBracket + 1, closeAnnoBracket - openAnnoBracket - 1);
              pairs.clear ();

              std::string::size_type start = 0;
              std::string::size_type end   = 0;
              do
              {
                end = annotations.find ('"', start);
                if (end != std::string::npos)
                {
                  end = annotations.find ('"', end + 1);

                  if (start != std::string::npos &&
                      end   != std::string::npos)
                  {
                    pairs.push_back (annotations.substr (start, end - start + 1));
                    start = end + 2;
                  }
                }
              }
              while (start != std::string::npos &&
                     end   != std::string::npos);

              for (size_t i = 0; i < pairs.size (); ++i)
              {
                std::string pair = pairs[i];
                std::string::size_type colon = pair.find (":");
                if (colon != std::string::npos)
                {
                  std::string name = pair.substr (0, colon);
                  std::string value = pair.substr (colon + 2, pair.length () - colon - 3);
                  mAnnotations[::atoi (name.c_str ())] = value;
                }
              }

              mDescription = line.substr (closeAnnoBracket + 2, std::string::npos);
            }
            else
              throw std::string ("Missing annotation brackets.");
          }
          else
            throw std::string ("Missing attribute brackets.");
        }
        else
          throw std::string ("Missing tag brackets.");
      }
      else
        throw std::string ("Line too short.");
    }
*/
    break;

  default:
    throw std::string ("Unrecognized task file format.");
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string T2::composeCSV ()
{
  throw std::string ("unimplemented T2::composeCSV");
  return "";
}

////////////////////////////////////////////////////////////////////////////////
void T2::getAnnotations (std::vector <Att>& annotations) const
{
  annotations.clear ();

  Record::const_iterator ci;
  for (ci = this->begin (); ci != this->end (); ++ci)
    if (ci->first.substr (0, 11) == "annotation_")
      annotations.push_back (ci->second);
}

////////////////////////////////////////////////////////////////////////////////
void T2::setAnnotations (const std::vector <Att>& annotations)
{
  // Erase old annotations.
  Record::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    if (i->first.substr (0, 11) == "annotation_")
      this->erase (i);

  std::vector <Att>::const_iterator ci;
  for (ci = annotations.begin (); ci != annotations.end (); ++ci)
    (*this)[ci->name ()] = *ci;
}

////////////////////////////////////////////////////////////////////////////////
// The timestamp is part of the name:
//    annotation_1234567890:"..."
//
void T2::addAnnotation (const std::string& description)
{
  std::stringstream s;
  s << "annotation_" << time (NULL);

  (*this)[s.str ()] = Att (s.str (), description);
}

////////////////////////////////////////////////////////////////////////////////
void T2::addTag (const std::string& tag)
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ',');

  if (std::find (tags.begin (), tags.end (), tag) == tags.end ())
  {
    tags.push_back (tag);
    std::string combined;
    join (combined, ",", tags);
    set ("tags", combined);
  }
}

////////////////////////////////////////////////////////////////////////////////
void T2::removeTag (const std::string& tag)
{
  std::vector <std::string> tags;
  split (tags, get ("tags"), ',');

  std::vector <std::string>::iterator i;
  i = std::find (tags.begin (), tags.end (), tag);
  if (i != tags.end ())
  {
    tags.erase (i);
    std::string combined;
    join (combined, ",", tags);
    set ("tags", combined);
  }
}

////////////////////////////////////////////////////////////////////////////////
bool T2::validate () const
{
  // TODO Verify until > due
  // TODO Verify entry < until, due, start, end
  return true;
}

////////////////////////////////////////////////////////////////////////////////
int T2::determineVersion (const std::string& line)
{
  // Version 2 looks like:
  //
  //   uuid status [tags] [attributes] description\n
  //
  // Where uuid looks like:
  //
  //   27755d92-c5e9-4c21-bd8e-c3dd9e6d3cf7
  //
  // Scan for the hyphens in the uuid, the following space, and a valid status
  // character.
  if (line[8]  == '-' &&
      line[13] == '-' &&
      line[18] == '-' &&
      line[23] == '-' &&
      line[36] == ' ' &&
      (line[37] == '-' || line[37] == '+' || line[37] == 'X' || line[37] == 'r'))
  {
    // Version 3 looks like:
    //
    //   uuid status [tags] [attributes] [annotations] description\n
    //
    // Scan for the number of [] pairs.
    std::string::size_type tagAtts  = line.find ("] [", 0);
    std::string::size_type attsAnno = line.find ("] [", tagAtts + 1);
    std::string::size_type annoDesc = line.find ("] ",  attsAnno + 1);
    if (tagAtts  != std::string::npos &&
        attsAnno != std::string::npos &&
        annoDesc != std::string::npos)
      return 3;
    else
      return 2;
  }

  // Version 1 looks like:
  //
  //   [tags] [attributes] description\n
  //   X [tags] [attributes] description\n
  //
  // Scan for the first character being either the bracket or X.
  else if ((line[0] == '[' && line[line.length () - 1] != ']') ||
           line.find ("X [") == 0)
    return 1;

  // Version 4 looks like:
  //
  //   [name:"value" ...]
  //
  // Scan for [, ] and :".
  if (line[0] == '[' &&
      line[line.length () - 1] == ']' &&
      line.find (":\"") != std::string::npos)
    return 4;

  // Version 5?
  //
  // Fortunately, with the hindsight that will come with version 5, the
  // identifying characteristics of 1, 2, 3 and 4 may be modified such that if 5
  // has a UUID followed by a status, then there is still a way to differentiate
  // between 2, 3, 4 and 5.
  //
  // The danger is that a version 3 binary reads and misinterprets a version 4
  // file.  This is why it is a good idea to rely on an explicit version
  // declaration rather than chance positioning.

  // Zero means 'no idea'.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
