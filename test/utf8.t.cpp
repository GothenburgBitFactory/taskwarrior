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
#include <utf8.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (33);

  std::string ascii_text            = "This is a test";
  std::string utf8_text             = "más sábado miércoles";
  std::string utf8_wide_text        = "改变各种颜色";

  std::string ascii_text_color      = "This [1mis[0m a test";
  std::string utf8_text_color       = "más [1msábado[0m miércoles";
  std::string utf8_wide_text_color  = "改[1m变各种[0m颜色";

  // unsigned int utf8_codepoint (const std::string&);
  t.is ((int) utf8_codepoint ("\\u0020"),              32, "\\u0020 --> ' '");
  t.is ((int) utf8_codepoint ("U+0020"),               32, "U+0020 --> ' '");

  // TODO unsigned int utf8_next_char (const std::string&, std::string::size_type&);
  // TODO std::string utf8_character (unsigned int);
  // TODO int utf8_sequence (unsigned int);

  // unsigned int utf8_length (const std::string&);
  t.is ((int) utf8_length (ascii_text),                14, "ASCII utf8_length");
  t.is ((int) utf8_length (utf8_text),                 20, "UTF8 utf8_length");
  t.is ((int) utf8_length (utf8_wide_text),             6, "UTF8 wide utf8_length");

  // unsigned int utf8_width (const std::string&);
  t.is ((int) utf8_width (ascii_text),                 14, "ASCII utf8_width");
  t.is ((int) utf8_width (utf8_text),                  20, "UTF8 utf8_width");
  t.is ((int) utf8_width (utf8_wide_text),             12, "UTF8 wide utf8_width");

  // unsigned int utf8_text_length (const std::string&);
  t.is ((int) utf8_text_length (ascii_text_color),     14, "ASCII utf8_text_length");
  t.is ((int) utf8_text_length (utf8_text_color),      20, "UTF8 utf8_text_length");
  t.is ((int) utf8_text_length (utf8_wide_text_color),  6, "UTF8 wide utf8_text_length");

  // unsigned int utf8_text_width (const std::string&);
  t.is ((int) utf8_text_width (ascii_text_color),      14, "ASCII utf8_text_width");
  t.is ((int) utf8_text_width (utf8_text_color),       20, "UTF8 utf8_text_width");
  t.is ((int) utf8_text_width (utf8_wide_text_color),  12, "UTF8 wide utf8_text_width");

  // const std::string utf8_substr (const std::string&, unsigned int, unsigned int length = 0);
  t.is (utf8_substr (ascii_text, 0, 2),                    "Th", "ASCII utf8_substr");
  t.is (utf8_substr (utf8_text, 0, 2),                     "má", "UTF8 utf8_substr");
  t.is (utf8_substr (utf8_wide_text, 0, 2),                "改变", "UTF8 wide utf8_substr");

  // int mk_wcwidth (wchar_t);
  t.is (mk_wcwidth ('a'),                               1, "mk_wcwidth U+0061 --> 1");
  t.is (mk_wcwidth (0x5149),                            2, "mk_wcwidth U+5149 --> 2");
  t.is (mk_wcwidth (0x9a8c),                            2, "mk_wcwidth U+9a8c --> 2");
  t.is (mk_wcwidth (0x4e70),                            2, "mk_wcwidth U+4e70 --> 2");
  t.is (mk_wcwidth (0x94b1),                            2, "mk_wcwidth U+94b1 --> 2");
  t.is (mk_wcwidth (0x5305),                            2, "mk_wcwidth U+5305 --> 2");
  t.is (mk_wcwidth (0x91cd),                            2, "mk_wcwidth U+91cd --> 2");
  t.is (mk_wcwidth (0x65b0),                            2, "mk_wcwidth U+65b0 --> 2");
  t.is (mk_wcwidth (0x8bbe),                            2, "mk_wcwidth U+8bbe --> 2");
  t.is (mk_wcwidth (0x8ba1),                            2, "mk_wcwidth U+8ba1 --> 2");
  t.is (mk_wcwidth (0x5411),                            2, "mk_wcwidth U+5411 --> 2");
  t.is (mk_wcwidth (0x4e0a),                            2, "mk_wcwidth U+4e0a --> 2");
  t.is (mk_wcwidth (0x4e0b),                            2, "mk_wcwidth U+4e0b --> 2");
  t.is (mk_wcwidth (0x7bad),                            2, "mk_wcwidth U+7bad --> 2");
  t.is (mk_wcwidth (0x5934),                            2, "mk_wcwidth U+5934 --> 2");
  t.is (mk_wcwidth (0xff0c),                            2, "mk_wcwidth U+ff0c --> 2"); // comma

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
