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

#include <iostream>
#include <utf8.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (4);

  std::string ascii_text = "This is a test";
  std::string utf8_text  = "más sábado miércoles";

  // TODO unsigned int utf8_codepoint (const std::string&);
  // TODO unsigned int utf8_next_char (const std::string&, std::string::size_type&);
  // TODO std::string utf8_character (unsigned int);
  // TODO int utf8_sequence (unsigned int);

  // unsigned int utf8_length (const std::string&);
  t.is (utf8_length (ascii_text), 14, "ASCII utf8_length");
  t.is (utf8_length (utf8_text),  20, "UTF8 utf8_length");

  // TODO unsigned int utf8_text_length (const std::string&);

  // const std::string utf8_substr (const std::string&, unsigned int, unsigned int length = 0);
  t.is (utf8_substr (ascii_text, 0, 2), "Th", "ASCII utf8_substr");
  t.is (utf8_substr (utf8_text, 0, 2),  "má", "UTF8 utf8_substr");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
