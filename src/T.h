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
#ifndef INCLUDED_T
#define INCLUDED_T

#include <string>
#include <vector>
#include <map>

// Length of longest line.
#define T_LINE_MAX 32768

class T
{
public:
  enum status   {pending, completed, deleted, recurring};

  T ();                    // Default constructor
  T (const std::string&);  // Initialize by parsing storage format
  T (const T&);            // Copy constructor
  T& operator= (const T&); // Assignment operator
  ~T ();                   // Destructor

  std::string getUUID () const                         { return mUUID; }
  void setUUID (const std::string& uuid)               { mUUID = uuid; }

  int getId () const                                   { return mId; }
  void setId (int id)                                  { mId = id; }
  std::vector <int> getAllIds () const                 { return mSequence; }
  void addId (int id)                                  { mSequence.push_back (id); }

  status getStatus () const                            { return mStatus; }
  void setStatus (status s)                            { mStatus = s; }

  const std::string getDescription () const            { return mDescription; }
  void setDescription (const std::string& description) { mDescription = description; }
  int getAnnotationCount () const                      { return mAnnotations.size (); }

  void getSubstitution (std::string&, std::string&, bool&) const;
  void setSubstitution (const std::string&, const std::string&, bool);

  bool hasTag (const std::string&) const;

  void getRemoveTags (std::vector<std::string>&); // SPECIAL
  void addRemoveTag (const std::string&);         // SPECIAL

  int getTagCount () const;
  void getTags (std::vector<std::string>&) const;
  void addTag (const std::string&);
  void addTags (const std::vector <std::string>&);
  void removeTag (const std::string&);
  void removeTags ();
  void getAttributes (std::map<std::string, std::string>&);
  const std::string getAttribute (const std::string&);
  void setAttribute (const std::string&, const std::string&);
  void setAttributes (const std::map <std::string, std::string>&);
  void removeAttribute (const std::string&);
  void removeAttributes ();

  void getAnnotations (std::map <time_t, std::string>&) const;
  void setAnnotations (const std::map <time_t, std::string>&);
  void addAnnotation (const std::string&);
  bool sequenceContains (int) const;

  const std::string compose () const;
  const std::string composeCSV ();
  void parse (const std::string&);
  bool validate () const;

private:
  int determineVersion (const std::string&);

private:
  status                             mStatus;
  std::string                        mUUID;
  int                                mId;
  std::vector <int>                  mSequence;
  std::string                        mDescription;
  std::vector<std::string>           mTags;
  std::vector<std::string>           mRemoveTags;
  std::map<std::string, std::string> mAttributes;
  std::string                        mFrom;
  std::string                        mTo;
  bool                               mGlobal;
  std::map <time_t, std::string>     mAnnotations;
};

#endif
////////////////////////////////////////////////////////////////////////////////
