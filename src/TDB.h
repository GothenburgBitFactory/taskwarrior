////////////////////////////////////////////////////////////////////////////////
// Copyright 2007, 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_TDB
#define INCLUDED_TDB

#include <vector>
#include <string>
#include "T.h"

class TDB
{
public:
  TDB ();
  ~TDB ();

  void dataDirectory    (const std::string&);
  bool allT             (std::vector <T>&) const;
  bool pendingT         (std::vector <T>&) const;
  bool allPendingT      (std::vector <T>&) const;
  bool completedT       (std::vector <T>&) const;
  bool allCompletedT    (std::vector <T>&) const;
  bool deleteT          (const T&) const;
  bool completeT        (const T&) const;
  bool addT             (const T&) const;
  bool modifyT          (const T&) const;
  bool logRead          (std::vector <std::string>&) const;
  bool logCommand       (int, char**) const;
  int gc                () const;

private:
  bool lock             (FILE*) const;
  bool overwritePending (std::vector <T>&) const;
  bool writePending     (const T&) const;
  bool writeCompleted   (const T&) const;
  bool readLockedFile   (const std::string&, std::vector <std::string>&) const;

private:
  std::string mPendingFile;
  std::string mCompletedFile;
  std::string mLogFile;
};

#endif
////////////////////////////////////////////////////////////////////////////////
