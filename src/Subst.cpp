////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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

#include "Subst.h"
#include "Nibbler.h"
#include "Context.h"
#include "text.h"
#include "i18n.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Subst::Subst ()
: mFrom ("")
, mTo ("")
, mGlobal (false)
{
}

////////////////////////////////////////////////////////////////////////////////
Subst::Subst (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Subst::Subst (const Subst& other)
{
  mFrom   = other.mFrom;
  mTo     = other.mTo;
  mGlobal = other.mGlobal;
}

////////////////////////////////////////////////////////////////////////////////
Subst& Subst::operator= (const Subst& other)
{
  if (this != &other)
  {
    mFrom   = other.mFrom;
    mTo     = other.mTo;
    mGlobal = other.mGlobal;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Subst::~Subst ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Subst::valid (const std::string& input) const
{
  std::string ignored;
  Nibbler n (input);
  if (n.skip     ('/')            &&
      n.getUntil ('/', ignored)   &&
      n.skip     ('/')            &&
      n.getUntil ('/', ignored)   &&
      n.skip     ('/'))
  {
    n.skip ('g');
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Subst::parse (const std::string& input)
{
  Nibbler n (input);
  if (n.skip     ('/')        &&
      n.getUntil ('/', mFrom) &&
      n.skip     ('/')        &&
      n.getUntil ('/', mTo)   &&
      n.skip     ('/'))
  {
      mGlobal = n.skip ('g');

    if (mFrom == "")
      throw context.stringtable.get (SUBST_EMPTY,
                                     "Cannot substitute an empty string.");

    if (!n.depleted ())
      throw context.stringtable.get (SUBST_BAD_CHARS,
                                     "Unrecognized character(s) at end of substitution.");
  }
  else
    throw context.stringtable.get (SUBST_MALFORMED,
                                   "Malformed substitution.");
}

////////////////////////////////////////////////////////////////////////////////
void Subst::apply (
  std::string& description,
  std::vector <Att>& annotations) const
{
  std::string::size_type pattern;
  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  if (mFrom != "")
  {
    if (mGlobal)
    {
      // Perform all subs on description.
      int counter = 0;
      pattern = 0;

      while ((pattern = find (description, mFrom, pattern, sensitive)) != std::string::npos)
      {
        description.replace (pattern, mFrom.length (), mTo);
        pattern += mTo.length ();

        if (++counter > 1000)
          throw ("Terminated substitution because more than a thousand changes were made - infinite loop protection.");
      }

      // Perform all subs on annotations.
      counter = 0;
      pattern = 0;
      std::vector <Att>::iterator i;
      for (i = annotations.begin (); i != annotations.end (); ++i)
      {
        std::string annotation = i->value ();
        while ((pattern = find (annotation, mFrom, pattern, sensitive)) != std::string::npos)
        {
          annotation.replace (pattern, mFrom.length (), mTo);
          pattern += mTo.length ();

          i->value (annotation);

          if (++counter > 1000)
            throw ("Terminated substitution because more than a thousand changes were made - infinite loop protection.");
        }
      }
    }
    else
    {
      // Perform first description substitution.
      if ((pattern = find (description, mFrom, sensitive)) != std::string::npos)
        description.replace (pattern, mFrom.length (), mTo);

      // Failing that, perform the first annotation substitution.
      else
      {
        std::vector <Att>::iterator i;
        for (i = annotations.begin (); i != annotations.end (); ++i)
        {
          std::string annotation = i->value ();
          if ((pattern = find (annotation, mFrom, sensitive)) != std::string::npos)
          {
            annotation.replace (pattern, mFrom.length (), mTo);
            i->value (annotation);
            break;
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Subst::clear ()
{
  mFrom = "";
  mTo = "";
  mGlobal = false;
}

////////////////////////////////////////////////////////////////////////////////
