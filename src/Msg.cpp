////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
#include <Msg.h>
#include <text.h>

////////////////////////////////////////////////////////////////////////////////
Msg::Msg ()
: _payload ("")
{
  // All messages are marked with the version number, so that the messages may
  // be properly evaluated in context.
  _header["version"] = PACKAGE_STRING;
}

////////////////////////////////////////////////////////////////////////////////
Msg::Msg (const Msg& other)
: _header (other._header)
, _payload (other._payload)
{
}

////////////////////////////////////////////////////////////////////////////////
Msg& Msg::operator= (const Msg& other)
{
  if (this != &other)
  {
    _header  = other._header;
    _payload = other._payload;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Msg::operator== (const Msg& other) const
{
  return _header  == other._header &&
         _payload == other._payload;
}

////////////////////////////////////////////////////////////////////////////////
Msg::~Msg ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Msg::clear ()
{
  _header.clear ();
  _payload = "";
}

////////////////////////////////////////////////////////////////////////////////
void Msg::set (const std::string& name, const int value)
{
  _header[name] = format (value);
}

////////////////////////////////////////////////////////////////////////////////
void Msg::set (const std::string& name, const std::string& value)
{
  _header[name] = value;
}

////////////////////////////////////////////////////////////////////////////////
void Msg::set (const std::string& name, const double value)
{
  _header[name] = format (value);
}

////////////////////////////////////////////////////////////////////////////////
void Msg::setPayload (const std::string& payload)
{
  _payload = payload;
}

////////////////////////////////////////////////////////////////////////////////
std::string Msg::get (const std::string& name) const
{
  std::map <std::string, std::string>::const_iterator i;
  i = _header.find (name);
  if (i != _header.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string Msg::getPayload () const
{
  return _payload;
}

////////////////////////////////////////////////////////////////////////////////
void Msg::all (std::vector <std::string>& names) const
{
  std::map <std::string, std::string>::const_iterator i;
  for (i = _header.begin (); i != _header.end (); ++i)
    names.push_back (i->first);
}

////////////////////////////////////////////////////////////////////////////////
std::string Msg::serialize () const
{
  std::string output;

  std::map <std::string, std::string>::const_iterator i;
  for (i = _header.begin (); i != _header.end (); ++i)
    output += i->first + ": " + i->second + "\n";

  output += "\n" + _payload + "\n";

  return output;
}

////////////////////////////////////////////////////////////////////////////////
bool Msg::parse (const std::string& input)
{
  _header.clear ();
  _payload = "";

  std::vector <std::string> lines;
  split (lines, input.substr (0, input.size ()), '\n');

  std::vector <std::string>::iterator i;
  bool tripped = false;
  for (i = lines.begin (); i != lines.end (); ++i)
  {
    if (*i == "")
      tripped = true;
    else if (tripped)
      _payload += *i + "\n";
    else
    {
      std::string::size_type delim = i->find (": ");
      if (delim != std::string::npos)
        _header[i->substr (0, delim)] = i->substr (delim + 2);
      else
        throw std::string ("ERROR: Malformed message header '") + *i + "'";
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
