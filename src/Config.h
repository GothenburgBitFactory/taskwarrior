////////////////////////////////////////////////////////////////////////////////
// Copyright 2005 - 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_CONFIG
#define INCLUDED_CONFIG

#include <map>
#include <vector>
#include <string>

class Config : public std::map <std::string, std::string>
{
public:
  Config ();
  Config (const std::string&);

  bool load (const std::string&);
  void createDefault (const std::string&);

  const std::string& get (const char*);
  const std::string& get (const char*, const char*);
  const std::string& get (const std::string&);
  const std::string& get (const std::string&, const std::string&);
  bool get (const std::string&, bool);
  int get (const std::string&, const int);
  double get (const std::string&, const double);
  void get (const std::string&, std::vector <std::string>&);
  void set (const std::string&, const int);
  void set (const std::string&, const double);
  void set (const std::string&, const std::string&);
  void set (const std::string&, const std::vector <std::string>&);
  void all (std::vector <std::string>&);
};

#endif

////////////////////////////////////////////////////////////////////////////////
