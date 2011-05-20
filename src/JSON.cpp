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

#include <iostream> // TODO Remove.
#include <text.h>
#include <utf8.h>
#include <JSON.h>

////////////////////////////////////////////////////////////////////////////////
json::value* json::value::parse (Nibbler& nibbler)
{
  json::value* v;
  if ((v = json::object::parse        (nibbler)) ||
      (v = json::array::parse         (nibbler)) ||
      (v = json::string::parse        (nibbler)) ||
      (v = json::number::parse        (nibbler)) ||
      (v = json::literal::parse       (nibbler)))
    return v;

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
json::jtype json::value::type ()
{
  return json::j_value;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::value::dump ()
{
  return "<value>";
}

////////////////////////////////////////////////////////////////////////////////
json::string::string (const std::string& other)
{
  *this = other;
}

////////////////////////////////////////////////////////////////////////////////
json::string* json::string::parse (Nibbler& nibbler)
{
  std::string value;
  if (nibbler.getQuoted ('"', value, false))
  {
    json::string* s = new json::string ();
    *(std::string*)s = value;
    return s;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
json::jtype json::string::type ()
{
  return json::j_string;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::string::dump ()
{
  return std::string ("\"") + (std::string) *this + "\"";
}

////////////////////////////////////////////////////////////////////////////////
json::number* json::number::parse (Nibbler& nibbler)
{
  int i;
  double d;
  if (nibbler.getNumber (d))
  {
    json::number* s = new json::number ();
    s->_dvalue = d;
    return s;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
json::jtype json::number::type ()
{
  return json::j_number;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::number::dump ()
{
  return format (_dvalue);
}

////////////////////////////////////////////////////////////////////////////////
json::number::operator double () const
{
  return _dvalue;
}

////////////////////////////////////////////////////////////////////////////////
json::literal* json::literal::parse (Nibbler& nibbler)
{
  if (nibbler.getLiteral ("null"))
  {
    json::literal* s = new json::literal ();
    s->_lvalue = nullvalue;
    return s;
  }
  else if (nibbler.getLiteral ("false"))
  {
    json::literal* s = new json::literal ();
    s->_lvalue = falsevalue;
    return s;
  }
  else if (nibbler.getLiteral ("true"))
  {
    json::literal* s = new json::literal ();
    s->_lvalue = truevalue;
    return s;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
json::jtype json::literal::type ()
{
  return json::j_literal;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::literal::dump ()
{
       if (_lvalue == nullvalue)  return "null";
  else if (_lvalue == falsevalue) return "false";
  else                            return "true";
}

////////////////////////////////////////////////////////////////////////////////
json::array::~array ()
{
  std::vector <json::value*>::iterator i;
  for (i  = ((std::vector <json::value*>*)this)->begin ();
       i != ((std::vector <json::value*>*)this)->end ();
       ++i)
    delete *i;
}

////////////////////////////////////////////////////////////////////////////////
json::array* json::array::parse (Nibbler& nibbler)
{
  Nibbler n (nibbler);
  n.skipWS ();
  if (n.skip ('['))
  {
    n.skipWS ();

    json::array* arr = new json::array ();

    json::value* value;
    if (value = json::value::parse (n))
    {
      arr->push_back (value);
      value = NULL; // Not a leak.  Looks like a leak.
      n.skipWS ();
      while (n.skip (','))
      {
        n.skipWS ();

        if (value = json::value::parse (n))
        {
          arr->push_back (value);
          n.skipWS ();
        }
        else
        {
          delete arr;
          return NULL;
        }
      }
    }
    if (n.skip (']'))
    {
      nibbler = n;
      return arr;
    }

    delete arr;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
json::jtype json::array::type ()
{
  return json::j_array;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::array::dump ()
{
  std::string output;
  output += "[";

  std::vector <json::value*>::iterator i;
  for (i  = ((std::vector <json::value*>*)this)->begin ();
       i != ((std::vector <json::value*>*)this)->end ();
       ++i)
  {
    if (i != ((std::vector <json::value*>*)this)->begin ())
      output += ",";

    output += (*i)->dump ();
  }

  output += "]";
  return output;
}

////////////////////////////////////////////////////////////////////////////////
json::object::~object ()
{
  std::map <std::string, json::value*>::iterator i;
  for (i  = ((std::map <std::string, json::value*>*)this)->begin ();
       i != ((std::map <std::string, json::value*>*)this)->end ();
       ++i)
    delete i->second;
}

////////////////////////////////////////////////////////////////////////////////
json::object* json::object::parse (Nibbler& nibbler)
{
  Nibbler n (nibbler);
  n.skipWS ();
  if (n.skip ('{'))
  {
    n.skipWS ();

    json::object* obj = new json::object ();

    std::string name;
    json::value* value;
    if (json::object::parse_pair (n, name, value))
    {
      obj->insert (std::pair <std::string, json::value*> (name, value));
      value = NULL; // Not a leak.  Looks like a leak.

      n.skipWS ();
      while (n.skip (','))
      {
        n.skipWS ();

        if (json::object::parse_pair (n, name, value))
        {
          obj->insert (std::pair <std::string, json::value*> (name, value));
          n.skipWS ();
        }
        else
        {
          delete obj;
          return NULL;
        }
      }
    }

    if (n.skip ('}'))
    {
      nibbler = n;
      return obj;
    }

    delete obj;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
bool json::object::parse_pair (
  Nibbler& nibbler,
  std::string& name,
  json::value*& val)
{
  Nibbler n (nibbler);

  if (n.getQuoted ('"', name, false))
  {
    n.skipWS ();
    if (n.skip (':'))
    {
      n.skipWS ();
      if (val = json::value::parse (n))
      {
        nibbler = n;
        return true;
      }
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
json::jtype json::object::type ()
{
  return json::j_object;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::object::dump ()
{
  std::string output;
  output += "{";

  std::map <std::string, json::value*>::iterator i;
  for (i  = ((std::map <std::string, json::value*>*)this)->begin ();
       i != ((std::map <std::string, json::value*>*)this)->end ();
       ++i)
  {
    if (i != ((std::map <std::string, json::value*>*)this)->begin ())
      output += ",";

    output += "\"" + i->first + "\":";
    output += i->second->dump ();
  }

  output += "}";
  return output;
}

////////////////////////////////////////////////////////////////////////////////
json::value* json::parse (const std::string& input)
{
  json::value* root = NULL;

  Nibbler n (input);
  n.skipWS ();

       if (n.next () == '{') root = json::object::parse (n);
  else if (n.next () == '[') root = json::array::parse (n);
  else
    throw std::string ("Error: expected '{' or '[' at position ") +
          format ((int)n.cursor ());

  // Check for end condition.
  n.skipWS ();
  if (!n.depleted ())
  {
    delete root;
    throw std::string ("Error: extra characters found: ") + n.dump ();
  }

  return root;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::encode (const std::string& input)
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
std::string json::decode (const std::string& input)
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
