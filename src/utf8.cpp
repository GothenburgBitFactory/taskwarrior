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

#include <string>
#include <utf8.h>

////////////////////////////////////////////////////////////////////////////////
// Converts '0'     -> 0
//          '9'     -> 9
//          'a'/'A' -> 10
//          'f'/'F' -> 15
#define XDIGIT(x) ((x) >= '0' && (x) <= '9' ? ((x) - '0') : \
                   (x) >= 'a' && (x) <= 'f' ? ((x) + 10 - 'a') : \
                   (x) >= 'A' && (x) <= 'F' ? ((x) + 10 - 'A') : 0)

////////////////////////////////////////////////////////////////////////////////
// Note: Assumes 4-digit hex codepoints:
//         xxxx
//         \uxxxx
//         U+xxxx
unsigned int utf8_codepoint (const std::string& input)
{
  unsigned int codepoint = 0;
  int length = input.length ();

  // U+xxxx, \uxxxx
  if (length >= 6 &&
      ((input[0] == 'U'  && input[1] == '+') ||
       (input[0] == '\\' && input[1] == 'u')))
  {
    codepoint = XDIGIT (input[2]) << 12 |
                XDIGIT (input[3]) <<  8 |
                XDIGIT (input[4]) <<  4 |
                XDIGIT (input[5]);
  }
  else if (length >= 4)
  {
    codepoint = XDIGIT (input[0]) << 12 |
                XDIGIT (input[1]) <<  8 |
                XDIGIT (input[2]) <<  4 |
                XDIGIT (input[3]);
  }
  else
    throw std::string ("Invalid codepoint representation.");

  return codepoint;
}

////////////////////////////////////////////////////////////////////////////////
// Iterates along a UTF8 string.
unsigned int utf8_next_char (const std::string& input, std::string::size_type& i)
{
  // How many bytes in the sequence?
  int length = utf8_sequence (input[i]);

  // 0xxxxxxx -> 0xxxxxxx
  if (length == 1)
    return input[i++];

  // 110yyyyy 10xxxxxx -> 00000yyy yyxxxxxx
  if (length == 2)
    return ((input[i++] & 0x1F) << 6) +
            (input[i++] & 0x3F);

  // 1110zzzz 10yyyyyy 10xxxxxx -> zzzzyyyy yyxxxxxx
  if (length == 3)
    return ((input[i++] & 0xF)  << 12) +
           ((input[i++] & 0x3F) <<  6) +
            (input[i++] & 0x3F);

  // 11110www 10zzzzzz 10yyyyyy 10xxxxxx -> 000wwwzz zzzzyyyy yyxxxxxx
  if (length == 4)
    return ((input[i++] & 0x7)  << 18) +
           ((input[i++] & 0x3F) << 12) +
           ((input[i++] & 0x3F) <<  6) +
            (input[i++] & 0x3F);

  // Default: pretend as though it's a single character.
  // TODO Or should this throw?
  return input[i++];
}

////////////////////////////////////////////////////////////////////////////////
// http://en.wikipedia.org/wiki/UTF-8
std::string utf8_character (unsigned int codepoint)
{
  char sequence[5];

  // 0xxxxxxx -> 0xxxxxxx
  if (codepoint < 0x80)
  {
    sequence[0] = codepoint;
    sequence[1] = 0;
  }

  // 00000yyy yyxxxxxx -> 110yyyyy 10xxxxxx
  else if (codepoint < 0x800)
  {
    sequence[0] = 0xC0 | (codepoint & 0x7C0) >> 6;
    sequence[1] = 0x80 | (codepoint & 0x3F);
    sequence[2] = 0;
  }

  // zzzzyyyy yyxxxxxx -> 1110zzzz 10yyyyyy 10xxxxxx
  else if (codepoint < 0x10000)
  {
    sequence[0] = 0xE0 | (codepoint & 0xF000) >> 12;
    sequence[1] = 0x80 | (codepoint & 0xFC0)  >> 6;
    sequence[2] = 0x80 | (codepoint & 0x3F);
    sequence[3] = 0;
  }

  // 000wwwzz zzzzyyyy yyxxxxxx -> 11110www 10zzzzzz 10yyyyyy 10xxxxxx
  else if (codepoint < 0x110000)
  {
    sequence[0] = 0xF0 | (codepoint & 0x1C0000) >> 18;
    sequence[1] = 0x80 | (codepoint & 0x03F000) >> 12;
    sequence[2] = 0x80 | (codepoint & 0x0FC0)   >> 6;
    sequence[3] = 0x80 | (codepoint & 0x3F);
    sequence[4] = 0;
  }
  else
    throw std::string ("Invalid Unicode codepoint.");

  sequence[4] = '\0';
  return std::string (sequence);
}

////////////////////////////////////////////////////////////////////////////////
int utf8_sequence (unsigned int character)
{
  if ((character & 0xE0) == 0xC0)
    return 2;

  if ((character & 0xF0) == 0xE0)
    return 3;

  if ((character & 0xF8) == 0xF0)
    return 4;

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
int utf8_length (const std::string& str)
{
  int byteLength = str.length ();
  int charLength = byteLength;
  const char* data = str.data ();

  // Decrement the number of bytes for each byte that matches 0b10??????
  // this way only the first byte of any utf8 sequence is counted.
  for (int i = 0; i < byteLength; i++)
  {
    // Extract the first two bits and check whether they are 10
    if ((data[i] & 0xC0) == 0x80)
      charLength--;
  }

  return charLength;
}

////////////////////////////////////////////////////////////////////////////////
