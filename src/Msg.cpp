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
#include <Msg.h>
#include <Lexer.h>
#include <text.h>

////////////////////////////////////////////////////////////////////////////////
void Msg::set (const std::string& name, const std::string& value)
{
  _header[name] = value;
}

////////////////////////////////////////////////////////////////////////////////
std::string Msg::get (const std::string& name) const
{
  auto i = _header.find (name);
  if (i != _header.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void Msg::setPayload (const std::string& payload)
{
  _payload = payload;
}

////////////////////////////////////////////////////////////////////////////////
std::string Msg::getPayload () const
{
  return _payload;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Msg::all () const
{
  std::vector <std::string> names;
  for (auto& i : _header)
    names.push_back (i.first);

  return names;
}

////////////////////////////////////////////////////////////////////////////////
std::string Msg::serialize () const
{
  std::string output;
  for (auto& i : _header)
    output += i.first + ": " + i.second + "\n";

  output += "\n" + _payload + "\n";
  return output;
}

////////////////////////////////////////////////////////////////////////////////
bool Msg::parse (const std::string& input)
{
  _header.clear ();
  _payload = "";

  auto separator = input.find ("\n\n");
  if (separator == std::string::npos)
    throw std::string ("ERROR: Malformed message");

  // Parse header.
  std::vector <std::string> lines;
  split (lines, input.substr (0, separator), '\n');
  for (auto& i : lines)
  {
    auto delimiter = i.find (':');
    if (delimiter == std::string::npos)
      throw std::string ("ERROR: Malformed message header '") + i + "'";

    _header[Lexer::trim (i.substr (0, delimiter))] = Lexer::trim (i.substr (delimiter + 1));
  }

  // Parse payload.
  _payload = input.substr (separator + 2);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
