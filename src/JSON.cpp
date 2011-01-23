////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Paul Beckingham, Federico Hernandez.
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
#include <utf8.h>
#include <JSON.h>

////////////////////////////////////////////////////////////////////////////////
JSON::JSON ()
: root ("root")
{
}

////////////////////////////////////////////////////////////////////////////////
JSON::JSON (const std::string& input)
: root ("root")
{
  Nibbler n (input);
  if (!parseObject (&root, n))
    throw std::string ("Syntax error in request.");
}

////////////////////////////////////////////////////////////////////////////////
JSON::~JSON ()
{
}

////////////////////////////////////////////////////////////////////////////////
// \n   -> "\\n"
// \t   -> "\\t"
std::string JSON::encode (const std::string& input)
{
  std::string output;

  for (std::string::size_type i = 0; i < input.length (); ++i)
  {
    switch (input[i])
    {
    // Simple translations.
    case '"':  output += "\\\"";   break;
    case '\\': output += "\\\\";   break;
    case '/':  output += "\\/";    break;
    case '\b': output += "\\b";    break;
    case '\f': output += "\\f";    break;
    case '\n': output += "\\n";    break;
    case '\r': output += "\\r";    break;
    case '\t': output += "\\t";    break;

    // Default NOP.
    default:   output += input[i]; break;
    }
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string JSON::decode (const std::string& input)
{
  std::string output;

  for (unsigned int i = 0; i < input.length (); ++i)
  {
    if (input[i] == '\\')
    {
      ++i;
      switch (input[i])
      {
      // Simple translations.
      case '"':  output += '"';  break;
      case '\\': output += '\\'; break;
      case '/':  output += '/';  break;
      case 'b':  output += '\b'; break;
      case 'f':  output += '\f'; break;
      case 'n':  output += '\n'; break;
      case 'r':  output += '\r'; break;
      case 't':  output += '\t'; break;

      // Compose a UTF8 unicode character.
      case 'u':
        output += utf8_character (utf8_codepoint (input.substr (++i)));
        i += 3;
        break;

      // If it is an unrecognized seqeence, do nothing.
      default:
        output += '\\';
        output += input[i];
        break;
      }
    }
    else
      output += input[i];
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
Tree* JSON::tree ()
{
  return &root;
}

////////////////////////////////////////////////////////////////////////////////
// object
//   {}
//   { pair , ... }
bool JSON::parseObject (Tree* t, Nibbler& nibbler)
{
  Nibbler n (nibbler);
  n.skipWS ();

  if (n.skip ('{'))
  {
    n.skipWS ();

    Tree* node = new Tree ("node");
    if (parsePair (node, n))
    {
      t->addBranch (node);

      n.skipWS ();
      while (n.skip (','))
      {
        n.skipWS ();

        node = new Tree ("node");
        if (!parsePair (node, n))
        {
          delete node;
          return false;
        }

        t->addBranch (node);
        n.skipWS ();
      }
    }
    else
      delete node;

    if (n.skip ('}'))
    {
      n.skipWS ();
      nibbler = n;
      t->attribute ("type", "collection");
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// pair
//   string : value
bool JSON::parsePair (Tree* t, Nibbler& nibbler)
{
  Nibbler n (nibbler);

  std::string value;
  if (n.getQuoted ('"', value))
  {
    n.skipWS ();
    if (n.skip (':'))
    {
      n.skipWS ();
      if (parseValue (t, n))
      {
        nibbler = n;
        t->name (value);
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// array
//   []
//   [ value , ... ]
bool JSON::parseArray (Tree* t, Nibbler& nibbler)
{
  Nibbler n (nibbler);
  n.skipWS ();

  if (n.skip ('['))
  {
    n.skipWS ();

    Tree* node = new Tree ("node");
    if (parseValue (node, n))
    {
      t->addBranch (node);

      n.skipWS ();
      while (n.skip (','))
      {
        n.skipWS ();

        node = new Tree ("node");
        if (!parseValue (node, n))
        {
          delete node;
          return false;
        }

        t->addBranch (node);
        n.skipWS ();
      }
    }
    else
      delete node;

    if (n.skip (']'))
    {
      n.skipWS ();
      nibbler = n;
      t->attribute ("type", "list");
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// value
//   string
//   number
//   object
//   array
//   true
//   false
//   null
bool JSON::parseValue (Tree* t, Nibbler& nibbler)
{
  if (parseString (t, nibbler)     ||
      parseNumber (t, nibbler)     ||
      parseObject (t, nibbler)     ||
      parseArray  (t, nibbler)     ||
      nibbler.getLiteral ("true")  ||
      nibbler.getLiteral ("false") ||
      nibbler.getLiteral ("null"))
  {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// string
//   ""
//   " chars "
//
// chars
//   char
//   char chars
//
// char
//   any-Unicode-character-except-"-or-\-or-control-character
//   \"
//   \\     [extra text to de-confuse gcc]
//   \/
//   \b
//   \f
//   \n
//   \r
//   \t
//   \u four-hex-digits
bool JSON::parseString (Tree* t, Nibbler& nibbler)
{
  std::string value;
  if (nibbler.getQuoted ('"', value, false))
  {
    t->attribute ("type", "string");
    t->attribute ("value", value);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// number
//   int frac exp
//   int frac
//   int exp
//   int
bool JSON::parseNumber (Tree* t, Nibbler& nibbler)
{
  int i;
  double d;
  if (nibbler.getNumber (d))
  {
    t->attribute ("type", "number");
    t->attribute ("value", d);
    return true;
  }
  else if (nibbler.getInt (i))
  {
    t->attribute ("type", "number");
    t->attribute ("value", i);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
