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
#include <iostream>
#include <sstream>
#include <algorithm>
#include "task.h"
#include "T.h"

////////////////////////////////////////////////////////////////////////////////
// Default
T::T ()
{
  mUUID = uuid ();
  mStatus = pending;
  mId = 0;
  mTags.clear ();
  mAttributes.clear ();
  mDescription = "";
  mFrom = "";
  mTo = "";
  mGlobal = false;
  mAnnotations.clear ();
}

////////////////////////////////////////////////////////////////////////////////
// Initialize by parsing storage format
T::T (const std::string& line)
{
  parse (line);
}

////////////////////////////////////////////////////////////////////////////////
T::T (const T& other)
{
  mStatus      = other.mStatus;
  mUUID        = other.mUUID;
  mId          = other.mId;
  mDescription = other.mDescription;
  mTags        = other.mTags;
  mRemoveTags  = other.mRemoveTags;
  mAttributes  = other.mAttributes;
  mAnnotations = other.mAnnotations;
}

////////////////////////////////////////////////////////////////////////////////
T& T::operator= (const T& other)
{
  if (this != &other)
  {
    mStatus      = other.mStatus;
    mUUID        = other.mUUID;
    mId          = other.mId;
    mDescription = other.mDescription;
    mTags        = other.mTags;
    mRemoveTags  = other.mRemoveTags;
    mAttributes  = other.mAttributes;
    mAnnotations = other.mAnnotations;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
T::~T ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool T::hasTag (const std::string& tag) const
{
  std::vector <std::string>::const_iterator it = find (mTags.begin (), mTags.end (), tag);
  if (it != mTags.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// SPECIAL METHOD - DO NOT REMOVE
void T::getRemoveTags (std::vector<std::string>& all)
{
  all = mRemoveTags;
}

////////////////////////////////////////////////////////////////////////////////
// SPECIAL METHOD - DO NOT REMOVE
void T::addRemoveTag (const std::string& tag)
{
  if (tag.find (' ') != std::string::npos)
    throw std::string ("T::addRemoveTag - tags may not contain spaces");

  mRemoveTags.push_back (tag);
}

////////////////////////////////////////////////////////////////////////////////
int T::getTagCount () const
{
  return mTags.size ();
}

////////////////////////////////////////////////////////////////////////////////
void T::getTags (std::vector<std::string>& all) const
{
  all = mTags;
}

////////////////////////////////////////////////////////////////////////////////
void T::addTag (const std::string& tag)
{
  if (tag.find (' ') != std::string::npos)
    throw std::string ("T::addTag - tags may not contain spaces");

  if (tag[0] == '+')
  {
    if (! hasTag (tag.substr (1, std::string::npos)))
      mTags.push_back (tag.substr (1, std::string::npos));
  }
  else
  {
    if (! hasTag (tag))
      mTags.push_back (tag);
  }
}

////////////////////////////////////////////////////////////////////////////////
void T::addTags (const std::vector <std::string>& tags)
{
  for (size_t i = 0; i < tags.size (); ++i)
  {
    if (tags[i].find (' ') != std::string::npos)
      throw std::string ("T::addTags - tags may not contain spaces");

    if (tags[i][0] == '+')
    {
      if (! hasTag (tags[i].substr (1, std::string::npos)))
        mTags.push_back (tags[i].substr (1, std::string::npos));
    }
    else
    {
      if (! hasTag (tags[i]))
        mTags.push_back (tags[i]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void T::removeTag (const std::string& tag)
{
  std::vector <std::string> copy;
  for (size_t i = 0; i < mTags.size (); ++i)
    if (mTags[i] != tag)
      copy.push_back (mTags[i]);

  mTags = copy;
}

////////////////////////////////////////////////////////////////////////////////
void T::removeTags ()
{
  mTags.clear ();
}

////////////////////////////////////////////////////////////////////////////////
void T::getAttributes (std::map<std::string, std::string>& all)
{
  all = mAttributes;
}

////////////////////////////////////////////////////////////////////////////////
const std::string T::getAttribute (const std::string& name)
{
  if (mAttributes.find (name) != mAttributes.end ())
    return mAttributes[name];

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void T::setAttribute (const std::string& name, const std::string& value)
{
  if (name.find (' ') != std::string::npos)
    throw std::string ("An attribute name may not contain spaces");

  if (value.find (' ') != std::string::npos)
    throw std::string ("An attribute value may not contain spaces");

  mAttributes[name] = value;
}

////////////////////////////////////////////////////////////////////////////////
void T::setAttributes (const std::map <std::string, std::string>& attributes)
{
  foreach (i, attributes)
  {
    if (i->first.find (' ') != std::string::npos)
      throw std::string ("An attribute name may not contain spaces");

    if (i->second.find (' ') != std::string::npos)
      throw std::string ("An attribute value may not contain spaces");

    mAttributes[i->first] = i->second;
  }
}

////////////////////////////////////////////////////////////////////////////////
void T::removeAttributes ()
{
  mAttributes.clear ();
}

////////////////////////////////////////////////////////////////////////////////
void T::removeAttribute (const std::string& name)
{
  std::map <std::string, std::string> copy = mAttributes;
  mAttributes.clear ();
  foreach (i, copy)
   if (i->first != name)
     mAttributes[i->first] = i->second;
}

////////////////////////////////////////////////////////////////////////////////
void T::getSubstitution (
  std::string& from,
  std::string& to,
  bool& global) const
{
  from   = mFrom;
  to     = mTo;
  global = mGlobal;
}

////////////////////////////////////////////////////////////////////////////////
void T::setSubstitution (
  const std::string& from,
  const std::string& to,
  bool global)
{
  mFrom   = from;
  mTo     = to;
  mGlobal = global;
}

////////////////////////////////////////////////////////////////////////////////
void T::getAnnotations (std::map <time_t, std::string>& all) const
{
  all = mAnnotations;
}

////////////////////////////////////////////////////////////////////////////////
void T::setAnnotations (const std::map <time_t, std::string>& all)
{
  mAnnotations = all;
}

////////////////////////////////////////////////////////////////////////////////
void T::addAnnotation (const std::string& description)
{
  std::string sanitized = description;
  std::replace (sanitized.begin (), sanitized.end (), '"', '\'');
  std::replace (sanitized.begin (), sanitized.end (), '[', '(');
  std::replace (sanitized.begin (), sanitized.end (), ']', ')');
  mAnnotations[time (NULL)] = sanitized;
}

////////////////////////////////////////////////////////////////////////////////
// uuid status [tags] [attributes] [annotations] description
//
// uuid         \x{8}-\x{4}-\x{4}-\x{4}-\x{12}
// status       - + X r
// tags         \w+ \s ...
// attributes   \w+:\w+ \s ...
// description  .+
//
const std::string T::compose () const
{
  // UUID
  std::string line = mUUID + ' ';

  // Status
       if (mStatus == pending)   line += "- [";
  else if (mStatus == completed) line += "+ [";
  else if (mStatus == deleted)   line += "X [";
  else if (mStatus == recurring) line += "r [";

  // Tags
  for (size_t i = 0; i < mTags.size (); ++i)
  {
    line += (i > 0 ? " " : "");
    line += mTags[i];
  }

  line += "] [";

  // Attributes
  int count = 0;
  foreach (i, mAttributes)
  {
    line += (count > 0 ? " " : "");
    line += i->first + ":" + i->second;

    ++count;
  }

  line += "] [";

  // Annotations
  std::stringstream annotation;
  bool first = true;
  foreach (note, mAnnotations)
  {
    if (first)
      first = false;
    else
      annotation << " ";

    annotation << note->first << ":\"" << note->second << "\"";
  }
  line += annotation.str () + "] ";

  // Description
  line += mDescription;

  // EOL
  line += "\n";

  if (line.length () > T_LINE_MAX)
    throw std::string ("Line too long");

  return line;
}

////////////////////////////////////////////////////////////////////////////////
const std::string T::composeCSV ()
{
  // UUID
  std::string line = "'" + mUUID + "',";

  // Status
       if (mStatus == pending)   line += "'pending',";
  else if (mStatus == completed) line += "'completed',";
  else if (mStatus == deleted)   line += "'deleted',";
  else if (mStatus == recurring) line += "'recurring',";

  // Tags
  line += "'";
  for (size_t i = 0; i < mTags.size (); ++i)
  {
    line += (i > 0 ? " " : "");
    line += mTags[i];
  }

  line += "',";
  std::string value = mAttributes["entry"];
  line += value + ",";

  value = mAttributes["start"];
  if (value != "")
    line += value;
  line += ",";

  value = mAttributes["due"];
  if (value != "")
    line += value;
  line += ",";

  value = mAttributes["recur"];
  if (value != "")
    line += value;
  line += ",";

  value = mAttributes["end"];
  if (value != "")
    line += value;
  line += ",";

  value = mAttributes["project"];
  if (value != "")
    line += "'" + value + "'";
  line += ",";

  value = mAttributes["priority"];
  if (value != "")
    line += "'" + value + "'";
  line += ",";

  value = mAttributes["fg"];
  if (value != "")
    line += "'" + value + "'";
  line += ",";

  value = mAttributes["bg"];
  if (value != "")
    line += "'" + value + "'";
  line += ",";

  // Convert single quotes to double quotes, because single quotes are used to
  // delimit the values that need it.
  std::string clean = mDescription;
  std::replace (clean.begin (), clean.end (), '\'', '"');
  line += "'" + clean + "'\n";

  return line;
}

////////////////////////////////////////////////////////////////////////////////
// Read all file formats, write only the latest.
void T::parse (const std::string& line)
{
  switch (determineVersion (line))
  {
  // File format version 1, from 2006.11.27 - 2007.12.31
  case 1:
    {
      // Generate a UUID for forward support.
      mUUID = uuid ();

      if (line.length () > 6)       // ^\[\]\s\[\]\n
      {
        if (line[0] == 'X')
          setStatus (deleted);

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
               if (pair[1] != "")
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
    break;

  // File format version 2, from 2008.1.1 - 2009.3.23
  case 2:
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
    break;

  // File format version 3, from 2009.3.23
  case 3:
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
          }
          else
            throw std::string ("Missing attribute brackets");
        }
        else
          throw std::string ("Missing tag brackets");
      }
      else
        throw std::string ("Line too short");
    }
    break;

  default:
    throw std::string ("Unrecognized task file format.");
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////
// If this code is inaccurate, data corruption ensues.
int T::determineVersion (const std::string& line)
{
  // Version 1 looks like:
  //
  //   [tags] [attributes] description\n
  //   X [tags] [attributes] description\n
  //
  // Scan for the first character being either the bracket or X.
  if (line[0] == '[' ||
      line[0] == 'X')
    return 1;

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

  // Version 4?
  //
  // Fortunately, with the hindsight that will come with version 4, the
  // identifying characteristics of 1, 2 and 3 may be modified such that if 4
  // has a UUID followed by a status, then there is still a way to differentiate
  // between 2, 3 and 4.
  //
  // The danger is that a version 3 binary reads and misinterprets a version 4
  // file.  This is why it is a good idea to rely on an explicit version
  // declaration rather than chance positioning.

  // Zero means 'no idea'.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Expand this method into a full-blown task validation check.
bool T::validate () const
{
  // TODO Verify until > due
  // TODO Verify entry < until, due, start, end
  return true;
}

////////////////////////////////////////////////////////////////////////////////
