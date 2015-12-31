////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <JSON.h>
#include <text.h>
#include <i18n.h>
#include <utf8.h>

const char *json_encode[] = {
  "\x00", "\x01", "\x02", "\x03", "\x04", "\x05", "\x06", "\x07",
   "\\b",  "\\t",  "\\n", "\x0b",  "\\f",  "\\r", "\x0e", "\x0f",
  "\x10", "\x11", "\x12", "\x13", "\x14", "\x15", "\x16", "\x17",
  "\x18", "\x19", "\x1a", "\x1b", "\x1c", "\x1d", "\x1e", "\x1f",
  "\x20", "\x21", "\\\"", "\x23", "\x24", "\x25", "\x26", "\x27",
  "\x28", "\x29", "\x2a", "\x2b", "\x2c", "\x2d", "\x2e",  "\\/",
  "\x30", "\x31", "\x32", "\x33", "\x34", "\x35", "\x36", "\x37",
  "\x38", "\x39", "\x3a", "\x3b", "\x3c", "\x3d", "\x3e", "\x3f",
  "\x40", "\x41", "\x42", "\x43", "\x44", "\x45", "\x46", "\x47",
  "\x48", "\x49", "\x4a", "\x4b", "\x4c", "\x4d", "\x4e", "\x4f",
  "\x50", "\x51", "\x52", "\x53", "\x54", "\x55", "\x56", "\x57",
  "\x58", "\x59", "\x5a", "\x5b", "\\\\", "\x5d", "\x5e", "\x5f",
  "\x60", "\x61", "\x62", "\x63", "\x64", "\x65", "\x66", "\x67",
  "\x68", "\x69", "\x6a", "\x6b", "\x6c", "\x6d", "\x6e", "\x6f",
  "\x70", "\x71", "\x72", "\x73", "\x74", "\x75", "\x76", "\x77",
  "\x78", "\x79", "\x7a", "\x7b", "\x7c", "\x7d", "\x7e", "\x7f",
  "\x80", "\x81", "\x82", "\x83", "\x84", "\x85", "\x86", "\x87",
  "\x88", "\x89", "\x8a", "\x8b", "\x8c", "\x8d", "\x8e", "\x8f",
  "\x90", "\x91", "\x92", "\x93", "\x94", "\x95", "\x96", "\x97",
  "\x98", "\x99", "\x9a", "\x9b", "\x9c", "\x9d", "\x9e", "\x9f",
  "\xa0", "\xa1", "\xa2", "\xa3", "\xa4", "\xa5", "\xa6", "\xa7",
  "\xa8", "\xa9", "\xaa", "\xab", "\xac", "\xad", "\xae", "\xaf",
  "\xb0", "\xb1", "\xb2", "\xb3", "\xb4", "\xb5", "\xb6", "\xb7",
  "\xb8", "\xb9", "\xba", "\xbb", "\xbc", "\xbd", "\xbe", "\xbf",
  "\xc0", "\xc1", "\xc2", "\xc3", "\xc4", "\xc5", "\xc6", "\xc7",
  "\xc8", "\xc9", "\xca", "\xcb", "\xcc", "\xcd", "\xce", "\xcf",
  "\xd0", "\xd1", "\xd2", "\xd3", "\xd4", "\xd5", "\xd6", "\xd7",
  "\xd8", "\xd9", "\xda", "\xdb", "\xdc", "\xdd", "\xde", "\xdf",
  "\xe0", "\xe1", "\xe2", "\xe3", "\xe4", "\xe5", "\xe6", "\xe7",
  "\xe8", "\xe9", "\xea", "\xeb", "\xec", "\xed", "\xee", "\xef",
  "\xf0", "\xf1", "\xf2", "\xf3", "\xf4", "\xf5", "\xf6", "\xf7",
  "\xf8", "\xf9", "\xfa", "\xfb", "\xfc", "\xfd", "\xfe", "\xff"
};

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
  if (nibbler.getQuoted ('"', value))
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

  if (n.getQuoted ('"', name))
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
  output.reserve ((input.size () * 6) / 5);  // 20% increase.

  auto last = input.begin ();
  for (auto i = input.begin (); i != input.end (); ++i)
  {
    switch (*i)
    {
    // Simple translations.
    case  '"':
    case '\\':
    case  '/':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
      output.append (last, i);
      output += json_encode[(unsigned char)(*i)];
      last = i + 1;

    // Default NOP.
    }
  }

  output.append (last, input.end ());

  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string json::decode (const std::string& input)
{
  std::string output;
  output.reserve (input.size ());  // Same size.

  size_t pos = 0;

  while (pos < input.length ())
  {
    if (input[pos] == '\\')
    {
      ++pos;
      switch (input[pos])
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
          output += utf8_character (utf8_codepoint (input.substr (++pos)));
          pos += 3;
          break;

        // If it is an unrecognized sequence, do nothing.
        default:
          output += '\\';
          output += input[pos];
          break;
      }
      ++pos;
    }
    else
    {
      size_t next_backslash = input.find ('\\', pos);
      output.append (input, pos, next_backslash - pos);
      pos = next_backslash;
    }
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
