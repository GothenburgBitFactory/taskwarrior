////////////////////////////////////////////////////////////////////////////////
// Copyright 2006 - 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
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
  for (unsigned int i = 0; i < tags.size (); ++i)
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
  for (unsigned int i = 0; i < mTags.size (); ++i)
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
void T::getSubstitution (std::string& from, std::string& to) const
{
  from = mFrom;
  to = mTo;
}

////////////////////////////////////////////////////////////////////////////////
void T::setSubstitution (const std::string& from, const std::string& to)
{
  mFrom = from;
  mTo = to;
}

////////////////////////////////////////////////////////////////////////////////
// uuid status [tags] [attributes] description
//
// uuid         \x{8}-\x{4}-\x{4}-\x{4}-\x{12}
// status       - O X
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

  // Tags
  for (unsigned int i = 0; i < mTags.size (); ++i)
  {
    line += (i > 0 ? " " : "");
    line += mTags[i];
  }

  line += "] [";

  // Attributes
  int count = 0;
  foreach (i, mAttributes)
  {
    std::string converted = i->second;

    // Date attributes may need conversion to epoch.
    if (i->first == "due"   ||
        i->first == "start" ||
        i->first == "entry" ||
        i->first == "end")
    {
      if (i->second.find ("/") != std::string::npos)
        validDate (converted);
    }

    line += (count > 0 ? " " : "");
    line += i->first + ":" + converted;

    ++count;
  }

  line += "] ";

  // Description
  line += mDescription;
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

  // Tags
  line += "'";
  for (unsigned int i = 0; i < mTags.size (); ++i)
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

  line += "'" + mDescription + "'\n";

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

        unsigned int openTagBracket  = line.find ("[");
        unsigned int closeTagBracket = line.find ("]", openTagBracket);
        if (openTagBracket  != std::string::npos &&
            closeTagBracket != std::string::npos)
        {
          unsigned int openAttrBracket  = line.find ("[", closeTagBracket);
          unsigned int closeAttrBracket = line.find ("]", openAttrBracket);
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
             for (unsigned int i = 0; i <  pairs.size (); ++i)
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
    }
    break;

  // File format version 2, from 2008.1.1
  case 2:
    {
      if (line.length () > 46)       // ^.{36} . \[\] \[\] \n
      {
        mUUID = line.substr (0, 36);

        mStatus =   line[37] == '+' ? completed
                  : line[37] == 'X' ? deleted
                  :                  pending;

        unsigned int openTagBracket  = line.find ("[");
        unsigned int closeTagBracket = line.find ("]", openTagBracket);
        if (openTagBracket  != std::string::npos &&
            closeTagBracket != std::string::npos)
        {
          unsigned int openAttrBracket  = line.find ("[", closeTagBracket);
          unsigned int closeAttrBracket = line.find ("]", openAttrBracket);
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
             for (unsigned int i = 0; i <  pairs.size (); ++i)
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
    }
    break;

  default:
    throw std::string ();
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
      (line[37] == '-' || line[37] == '+' || line[37] == 'X'))
    return 2;

  // Version 3?
  //
  // Fortunately, with the hindsight that will come with version 3, the
  // identifying characteristics of 1 and 2 may be modified such that if 3 has
  // a UUID followed by a status, then there is still a way to differentiate
  // between 2 and 3.
  //
  // The danger is that a version 2 binary reads and misinterprets a version 2
  // file.  This is why it is a good idea to rely on an explicit version
  // declaration rather than chance positioning.

  // Zero means 'no idea'.
  return 0;
}

