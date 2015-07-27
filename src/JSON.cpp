////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <text.h>
#include <i18n.h>
#include <utf8.h>
#include <JSON.h>

////////////////////////////////////////////////////////////////////////////////
json::value* json::value::parse (Nibbler& nibbler)
{
  json::value* v;
  if ((v = json::object::parse  (nibbler)) ||
      (v = json::array::parse   (nibbler)) ||
      (v = json::string::parse  (nibbler)) ||
      (v = json::number::parse  (nibbler)) ||
      (v = json::literal::parse (nibbler)))
    return v;

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
json::jtype json::value::type ()
{
  return json::j_value;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::value::dump () const
{
  return "<value>";
}

////////////////////////////////////////////////////////////////////////////////
json::string::string (const std::string& other)
{
  _data = other;
}

////////////////////////////////////////////////////////////////////////////////
json::string* json::string::parse (Nibbler& nibbler)
{
  std::string value;
  if (nibbler.getQuoted ('"', value, false))
  {
    json::string* s = new json::string ();
    s->_data = value;
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
std::string json::string::dump () const
{
  return std::string ("\"") + _data + "\"";
}

////////////////////////////////////////////////////////////////////////////////
json::number* json::number::parse (Nibbler& nibbler)
{
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
std::string json::number::dump () const
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
std::string json::literal::dump () const
{
       if (_lvalue == nullvalue)  return "null";
  else if (_lvalue == falsevalue) return "false";
  else                            return "true";
}

////////////////////////////////////////////////////////////////////////////////
json::array::~array ()
{
  for (auto& i : _data)
    delete i;
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
    if ((value = json::value::parse (n)))
    {
      arr->_data.push_back (value);
      value = NULL; // Not a leak.  Looks like a leak.
      n.skipWS ();
      while (n.skip (','))
      {
        n.skipWS ();

        if ((value = json::value::parse (n)))
        {
          arr->_data.push_back (value);
          n.skipWS ();
        }
        else
        {
          delete arr;
          throw format (STRING_JSON_MISSING_VALUE, (int) n.cursor ());
        }
      }
    }

    if (n.skip (']'))
    {
      nibbler = n;
      return arr;
    }
    else
      throw format (STRING_JSON_MISSING_BRACKET, (int) n.cursor ());

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
std::string json::array::dump () const
{
  std::string output;
  output += "[";

  for (auto i = _data.begin (); i != _data.end (); ++i)
  {
    if (i != _data.begin ())
      output += ",";

    output += (*i)->dump ();
  }

  output += "]";
  return output;
}

////////////////////////////////////////////////////////////////////////////////
json::object::~object ()
{
  for (auto& i : _data)
    delete i.second;
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
      obj->_data.insert (std::pair <std::string, json::value*> (name, value));
      value = NULL; // Not a leak.  Looks like a leak.

      n.skipWS ();
      while (n.skip (','))
      {
        n.skipWS ();

        if (json::object::parse_pair (n, name, value))
        {
          obj->_data.insert (std::pair <std::string, json::value*> (name, value));
          n.skipWS ();
        }
        else
        {
          delete obj;
          throw format (STRING_JSON_MISSING_VALUE, (int) n.cursor ());
        }
      }
    }

    if (n.skip ('}'))
    {
      nibbler = n;
      return obj;
    }
    else
      throw format (STRING_JSON_MISSING_BRACE, (int) n.cursor ());

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
      if ((val = json::value::parse (n)))
      {
        nibbler = n;
        return true;
      }
      else
        throw format (STRING_JSON_MISSING_VALUE2, (int) n.cursor ());
    }
    else
      throw format (STRING_JSON_MISSING_COLON, (int) n.cursor ());
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
json::jtype json::object::type ()
{
  return json::j_object;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::object::dump () const
{
  std::string output;
  output += "{";

  for (auto i = _data.begin (); i != _data.end (); ++i)
  {
    if (i != _data.begin ())
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
    throw format (STRING_JSON_MISSING_OPEN, (int) n.cursor ());

  // Check for end condition.
  n.skipWS ();
  if (!n.depleted ())
  {
    delete root;
    throw format (STRING_JSON_EXTRA_CHARACTERS, (int) n.cursor ());
  }

  return root;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::encode (const std::string& input)
{
  std::string output;

  for (auto& i : input)
  {
    switch (i)
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
    default:   output += i; break;
    }
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Using a state machine would speed this up.
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

      // If it is an unrecognized sequence, do nothing.
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
