////////////////////////////////////////////////////////////////////////////////
// Copyright 2006 - 2007, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_T
#define INCLUDED_T

#include <string>
#include <vector>
#include <map>

// Length of longest line.
#define T_LINE_MAX 8192

class T
{
public:
  enum status   {pending, completed, deleted};

  T ();                    // Default constructor
  T (const std::string&);  // Initialize by parsing storage format
  T (const T&);            // Copy constructor
  T& operator= (const T&); // Assignment operator
  ~T ();                   // Destructor

  std::string getUUID () const                         { return mUUID; }
  void setUUID (const std::string& uuid)               { mUUID = uuid; }

  int getId () const                                   { return mId; }
  void setId (int id)                                  { mId = id; }

  status getStatus () const                            { return mStatus; }
  void setStatus (status s)                            { mStatus = s; }

  const std::string getDescription () const            { return mDescription; }
  void setDescription (const std::string& description) { mDescription = description; }

  void getSubstitution (std::string&, std::string&) const;
  void setSubstitution (const std::string&, const std::string&);

  bool hasTag (const std::string&) const;

  void getRemoveTags (std::vector<std::string>&); // SPECIAL
  void addRemoveTag (const std::string&);         // SPECIAL

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

  const std::string compose () const;
  const std::string composeCSV ();
  void parse (const std::string&);

private:
  int determineVersion (const std::string&);

private:
  status                             mStatus;
  std::string                        mUUID;
  int                                mId;
  std::string                        mDescription;
  std::vector<std::string>           mTags;
  std::vector<std::string>           mRemoveTags;
  std::map<std::string, std::string> mAttributes;

  std::string                        mFrom;
  std::string                        mTo;
};

#endif
////////////////////////////////////////////////////////////////////////////////
