////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <ctype.h>
#include <Lexer.h>
#include <ISO8601.h>
#include <Date.h>
#include <Duration.h>
#include <utf8.h>

static const std::string uuid_pattern = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
static const unsigned int uuid_min_length = 8;

std::string Lexer::dateFormat = "";
bool Lexer::isoEnabled = true;

////////////////////////////////////////////////////////////////////////////////
Lexer::Lexer (const std::string& text)
: _text (text)
, _cursor (0)
, _eos (text.size ())
{
}

////////////////////////////////////////////////////////////////////////////////
Lexer::~Lexer ()
{
}

////////////////////////////////////////////////////////////////////////////////
// When a Lexer object is constructed with a string, this method walks through
// the stream of low-level tokens.
bool Lexer::token (std::string& token, Lexer::Type& type)
{
  // Eat white space.
  while (isWhitespace (_text[_cursor]))
    utf8_next_char (_text, _cursor);

  // Terminate at EOS.
  if (isEOS ())
    return false;

  // The sequence is specific, and must follow these rules:
  //   - date < duration < uuid < identifier
  //   - uuid < hex < number
  //   - url < pair < identifier
  //   - hex < number
  //   - separator < tag < operator
  //   - path < substitution < pattern
  //   - set < number
  //   - word last
  if (isString       (token, type, "'\"") ||
      isDate         (token, type)        ||
      isDuration     (token, type)        ||
      isURL          (token, type)        ||
      isPair         (token, type)        ||
      isSet          (token, type)        ||
      isDOM          (token, type)        ||
      isUUID         (token, type)        ||
      isHexNumber    (token, type)        ||
      isNumber       (token, type)        ||
      isSeparator    (token, type)        ||
      isTag          (token, type)        ||
      isPath         (token, type)        ||
      isSubstitution (token, type)        ||
      isPattern      (token, type)        ||
      isOperator     (token, type)        ||
      isIdentifier   (token, type)        ||
      isWord         (token, type))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// This static method tokenizes the input and provides a vector of token/type
// results from a high-level lex.
std::vector <std::pair <std::string, Lexer::Type>> Lexer::tokens (
  const std::string& text)
{
  std::vector <std::pair <std::string, Lexer::Type>> all;
  std::string token;
  Lexer::Type type;
  Lexer l (text);
  while (l.token (token, type))
    all.push_back (std::pair <std::string, Lexer::Type> (token, type));

  return all;
}

////////////////////////////////////////////////////////////////////////////////
// This static method tokenizes the input, but discards the type information.
std::vector <std::string> Lexer::split (const std::string& text)
{
  std::vector <std::string> all;
  std::string token;
  Lexer::Type ignored;
  Lexer l (text);
  while (l.token (token, ignored))
    all.push_back (token);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
// No L10N - these are for internal purposes.
const std::string Lexer::typeName (const Lexer::Type& type)
{
  switch (type)
  {
  case Lexer::Type::uuid:         return "uuid";
  case Lexer::Type::number:       return "number";
  case Lexer::Type::hex:          return "hex";
  case Lexer::Type::string:       return "string";
  case Lexer::Type::url:          return "url";
  case Lexer::Type::pair:         return "pair";
  case Lexer::Type::set:          return "set";
  case Lexer::Type::separator:    return "separator";
  case Lexer::Type::tag:          return "tag";
  case Lexer::Type::path:         return "path";
  case Lexer::Type::substitution: return "substitution";
  case Lexer::Type::pattern:      return "pattern";
  case Lexer::Type::op:           return "op";
  case Lexer::Type::dom:          return "dom";
  case Lexer::Type::identifier:   return "identifier";
  case Lexer::Type::word:         return "word";
  case Lexer::Type::date:         return "date";
  case Lexer::Type::duration:     return "duration";
  }

  return "unknown";
}

////////////////////////////////////////////////////////////////////////////////
// Complete Unicode whitespace list.
//
// http://en.wikipedia.org/wiki/Whitespace_character
// Updated 2013-11-18
// Static
//
// TODO This list should be derived from the Unicode database.
bool Lexer::isWhitespace (int c)
{
  return (c == 0x0020 ||   // space Common  Separator, space
          c == 0x0009 ||   // Common  Other, control  HT, Horizontal Tab
          c == 0x000A ||   // Common  Other, control  LF, Line feed
          c == 0x000B ||   // Common  Other, control  VT, Vertical Tab
          c == 0x000C ||   // Common  Other, control  FF, Form feed
          c == 0x000D ||   // Common  Other, control  CR, Carriage return
          c == 0x0085 ||   // Common  Other, control  NEL, Next line
          c == 0x00A0 ||   // no-break space  Common  Separator, space
          c == 0x1680 ||   // ogham space mark  Ogham Separator, space
          c == 0x180E ||   // mongolian vowel separator Mongolian Separator, space
          c == 0x2000 ||   // en quad Common  Separator, space
          c == 0x2001 ||   // em quad Common  Separator, space
          c == 0x2002 ||   // en space  Common  Separator, space
          c == 0x2003 ||   // em space  Common  Separator, space
          c == 0x2004 ||   // three-per-em space  Common  Separator, space
          c == 0x2005 ||   // four-per-em space Common  Separator, space
          c == 0x2006 ||   // six-per-em space  Common  Separator, space
          c == 0x2007 ||   // figure space  Common  Separator, space
          c == 0x2008 ||   // punctuation space Common  Separator, space
          c == 0x2009 ||   // thin space  Common  Separator, space
          c == 0x200A ||   // hair space  Common  Separator, space
          c == 0x2028 ||   // line separator  Common  Separator, line
          c == 0x2029 ||   // paragraph separator Common  Separator, paragraph
          c == 0x202F ||   // narrow no-break space Common  Separator, space
          c == 0x205F ||   // medium mathematical space Common  Separator, space
          c == 0x3000);    // ideographic space Common  Separator, space
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isAlpha (int c)
{
  return (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z');
}

////////////////////////////////////////////////////////////////////////////////
// Digits 0-9.
//
// TODO This list should be derived from the Unicode database.
bool Lexer::isDigit (int c)
{
  return c >= 0x30 && c <= 0x39;
}

////////////////////////////////////////////////////////////////////////////////
// Digits 0-9 a-f A-F.
bool Lexer::isHexDigit (int c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isIdentifierStart (int c)
{
  return c                          &&  // Include null character check.
         ! isWhitespace         (c) &&
         ! isDigit              (c) &&
         ! isSingleCharOperator (c) &&
         ! isPunctuation        (c);
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isIdentifierNext (int c)
{
  return c                          &&  // Include null character check.
         c != ':'                   &&  // Used in isPair.
         c != '='                   &&  // Used in isPair.
         ! isWhitespace         (c) &&
         ! isSingleCharOperator (c);
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isSingleCharOperator (int c)
{
  return c == '+' ||  // Addition
         c == '-' ||  // Subtraction or unary minus = ambiguous
         c == '*' ||  // Multiplication
         c == '/' ||  // Diviѕion
         c == '(' ||  // Precedence open parenthesis
         c == ')' ||  // Precedence close parenthesis
         c == '<' ||  // Less than
         c == '>' ||  // Greater than
         c == '^' ||  // Exponent
         c == '!' ||  // Unary not
         c == '%' ||  // Modulus
         c == '=' ||  // Partial match
         c == '~';    // Pattern match
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isDoubleCharOperator (int c0, int c1, int c2)
{
  return (c0 == '=' && c1 == '=')                        ||
         (c0 == '!' && c1 == '=')                        ||
         (c0 == '<' && c1 == '=')                        ||
         (c0 == '>' && c1 == '=')                        ||
         (c0 == 'o' && c1 == 'r' && isBoundary (c1, c2)) ||
         (c0 == '|' && c1 == '|')                        ||
         (c0 == '&' && c1 == '&')                        ||
         (c0 == '!' && c1 == '~');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isTripleCharOperator (int c0, int c1, int c2, int c3)
{
  return (c0 == 'a' && c1 == 'n' && c2 == 'd' && isBoundary (c2, c3)) ||
         (c0 == 'x' && c1 == 'o' && c2 == 'r' && isBoundary (c2, c3)) ||
         (c0 == '!' && c1 == '=' && c2 == '=');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isBoundary (int left, int right)
{
  // EOS
  if (right == '\0')                                 return true;

  // XOR
  if (isAlpha (left)       != isAlpha (right))       return true;
  if (isDigit (left)       != isDigit (right))       return true;
  if (isWhitespace (left)  != isWhitespace (right))  return true;

  // OR
  if (isPunctuation (left) || isPunctuation (right)) return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isPunctuation (int c)
{
  return isprint (c)   &&
         c != ' '      &&
         c != '@'      &&
         c != '#'      &&
         c != '$'      &&
         c != '_'      &&
         ! isDigit (c) &&
         ! isAlpha (c);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::dequote (std::string& input)
{
  int quote = input[0];
  size_t len = input.length ();
  if ((quote == '\'' || quote == '"') &&
      quote == input[len - 1])
  {
    input = input.substr (1, len - 2);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Detects characters in an input string that indicate quotes were required, or
// escapes, to get them past the shell.
bool Lexer::wasQuoted (const std::string& input)
{
  if (input.find_first_of (" \t()<>&~") != std::string::npos)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isEOS () const
{
  return _cursor >= _eos;
}

////////////////////////////////////////////////////////////////////////////////
// Converts '0'     -> 0
//          '9'     -> 9
//          'a'/'A' -> 10
//          'f'/'F' -> 15
int Lexer::hexToInt (int c)
{
       if (c >= '0' && c <= '9') return (c - '0');
  else if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
  else                           return (c - 'A' + 10);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::hexToInt (int c0, int c1)
{
  return (hexToInt (c0) << 4) + hexToInt (c1);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::hexToInt (int c0, int c1, int c2, int c3)
{
  return (hexToInt (c0) << 12) +
         (hexToInt (c1) << 8)  +
         (hexToInt (c2) << 4)  +
          hexToInt (c3);
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::string
//   '|"
//   [ U+XXXX | \uXXXX | \" | \' | \\ | \/ | \b | \f | \n | \r | \t | . ]
//   '|"
bool Lexer::isString (std::string& token, Lexer::Type& type, const std::string& quotes)
{
  std::size_t marker = _cursor;

/*
  if (readWord (_text, quotes, marker, token))
  {
    type = Lexer::Type::string;
    _cursor = marker;
    return true;
  }

  return false;
*/

  if (quotes.find (_text[marker]) != std::string::npos)
  {
    int quote = _text[marker];
    token = _text.substr (marker++, 1);

    int c;
    while ((c = _text[marker]))
    {
      // EOS.
      if (c == quote)
        break;

      // Unicode U+XXXX or \uXXXX codepoint.
      else if (_eos - marker >= 6 &&
               ((_text[marker + 0] == 'U' && _text[marker + 1] == '+') ||
                (_text[marker + 0] == '\\' && _text[marker + 1] == 'u')) &&
               isHexDigit (_text[marker + 2]) &&
               isHexDigit (_text[marker + 3]) &&
               isHexDigit (_text[marker + 4]) &&
               isHexDigit (_text[marker + 5]))
      {
        token += utf8_character (
                   hexToInt (
                     _text[marker + 2],
                     _text[marker + 3],
                     _text[marker + 4],
                     _text[marker + 5]));
        marker += 6;
      }

      // An escaped thing.
      else if (c == '\\')
      {
        c = _text[++marker];

        switch (c)
        {
        case '"':  token += (char) 0x22; ++marker; break;
        case '\'': token += (char) 0x27; ++marker; break;
        case '\\': token += (char) 0x5C; ++marker; break;
        case 'b':  token += (char) 0x08; ++marker; break;
        case 'f':  token += (char) 0x0C; ++marker; break;
        case 'n':  token += (char) 0x0A; ++marker; break;
        case 'r':  token += (char) 0x0D; ++marker; break;
        case 't':  token += (char) 0x09; ++marker; break;
        case 'v':  token += (char) 0x0B; ++marker; break;

        // This pass-through default case means that anythign can be escaped
        // harmlessly. In particular 'quote' is included, if it not one of the
        // above characters.
        default:   token += (char) c;    ++marker; break;
        }
      }

      // Ordinary character.
      else
        token += utf8_character (utf8_next_char (_text, marker));
    }

    if (_text[marker] == quote)
    {
      token += _text.substr (marker++, 1);
      type = Lexer::Type::string;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::date
//   <ISO8601d> | <Date>
bool Lexer::isDate (std::string& token, Lexer::Type& type)
{
  // Try an ISO date parse.
  if (Lexer::isoEnabled)
  {
    std::size_t iso_i = 0;
    ISO8601d iso;
    if (iso.parse (_text.substr (_cursor), iso_i))
    {
      type = Lexer::Type::date;
      token = _text.substr (_cursor, iso_i);
      _cursor += iso_i;
      return true;
    }
  }

  // Try a legacy rc.dateformat parse here.
  if (Lexer::dateFormat != "")
  {
    try
    {
      std::size_t legacy_i = 0;
      Date legacyDate (_text.substr (_cursor), legacy_i, Lexer::dateFormat, false, false);

      type = Lexer::Type::date;
      token = _text.substr (_cursor, legacy_i);
      _cursor += legacy_i;
      return true;
    }

    catch (...) { /* Never mind. */ }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::duration
//   <ISO8106p> | <Duration>
bool Lexer::isDuration (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;
  ISO8601p iso;

  std::string extractedToken;
  Lexer::Type extractedType;
  if (isOperator(extractedToken, extractedType))
  {
    _cursor = marker;
    return false;
  }

  marker = 0;
  if (iso.parse (_text.substr (_cursor), marker))
  {
    type = Lexer::Type::duration;
    token = _text.substr (_cursor, marker);
    _cursor += marker;
    return true;
  }

  Duration dur;
  if (dur.parse (_text.substr (_cursor), marker))
  {
    type = Lexer::Type::duration;
    token = _text.substr (_cursor, marker);
    _cursor += marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::uuid
//   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
//   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXX
//   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXX
//   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXX
//   ...
//   XXXXXXXX-XX
//   XXXXXXXX-X
//   XXXXXXXX-
//   XXXXXXXX
//   Followed only by EOS, whitespace, operator or list.
bool Lexer::isUUID (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  std::size_t i = 0;
  for (; i < 36 && marker + i < _eos; i++)
  {
    if (uuid_pattern[i] == 'x')
    {
      if (! isHexDigit (_text[marker + i]))
        break;
    }
    else if (uuid_pattern[i] != _text[marker + i])
      break;
  }

  if (i >= uuid_min_length)
  {
    token = _text.substr (_cursor, i);
    if (! isAllDigits (token))
    {
      type = Lexer::Type::uuid;
      _cursor += i;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::hex
//   0xX+
bool Lexer::isHexNumber (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  if (_eos - marker >= 3 &&
      _text[marker + 0] == '0' &&
      _text[marker + 1] == 'x')
  {
    marker += 2;

    while (isHexDigit (_text[marker]))
      ++marker;

    if (marker - _cursor > 2)
    {
      token = _text.substr (_cursor, marker - _cursor);
      type = Lexer::Type::hex;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::number
//   \d+
//   [ . \d+ ]
//   [ e|E [ +|- ] \d+ [ . \d+ ] ]
//   not followed by non-operator.
bool Lexer::isNumber (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  if (isDigit (_text[marker]))
  {
    ++marker;
    while (isDigit (_text[marker]))
      utf8_next_char (_text, marker);

    if (_text[marker] == '.')
    {
      ++marker;
      if (isDigit (_text[marker]))
      {
        ++marker;
        while (isDigit (_text[marker]))
          utf8_next_char (_text, marker);
      }
    }

    if (_text[marker] == 'e' ||
        _text[marker] == 'E')
    {
      ++marker;

      if (_text[marker] == '+' ||
          _text[marker] == '-')
        ++marker;

      if (isDigit (_text[marker]))
      {
        ++marker;
        while (isDigit (_text[marker]))
          utf8_next_char (_text, marker);

        if (_text[marker] == '.')
        {
          ++marker;
          if (isDigit (_text[marker]))
          {
            ++marker;
            while (isDigit (_text[marker]))
              utf8_next_char (_text, marker);
          }
        }
      }
    }

    // Lookahread: !<isWhitespace> | !<isSingleCharOperator>
    // If there is an immediately consecutive character, that is not an operator, fail.
    if (_eos > marker &&
        ! isWhitespace (_text[marker]) &&
        ! isSingleCharOperator (_text[marker]))
      return false;

    token = _text.substr (_cursor, marker - _cursor);
    type = Lexer::Type::number;
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::separator
//   --
bool Lexer::isSeparator (std::string& token, Lexer::Type& type)
{
  if (_eos - _cursor >= 2 &&
      _text[_cursor] == '-' &&
      _text[_cursor + 1] == '-')
  {
    _cursor += 2;
    type = Lexer::Type::separator;
    token = "--";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::url
//   http [s] :// ...
bool Lexer::isURL (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  if (_eos - _cursor > 9 &&    // length 'https://*'
      (_text[marker + 0] == 'h' || _text[marker + 0] == 'H') &&
      (_text[marker + 1] == 't' || _text[marker + 1] == 'T') &&
      (_text[marker + 2] == 't' || _text[marker + 2] == 'T') &&
      (_text[marker + 3] == 'p' || _text[marker + 3] == 'P'))
  {
    marker += 4;
    if (_text[marker + 0] == 's' || _text[marker + 0] == 'S')
      ++marker;

    if (_text[marker + 0] == ':' &&
        _text[marker + 1] == '/' &&
        _text[marker + 2] == '/')
    {
      marker += 3;

      while (marker < _eos &&
             ! isWhitespace (_text[marker]))
        utf8_next_char (_text, marker);

      token = _text.substr (_cursor, marker - _cursor);
      type = Lexer::Type::url;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::pair
//   <identifier> :  [ <string> | <word> ]
//   <identifier> =  [ <string> | <word> ]
//   <identifier> := [ <string> | <word> ]
bool Lexer::isPair (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  std::string ignoredToken;
  Lexer::Type ignoredType;
  if (isIdentifier (ignoredToken, ignoredType))
  {
    // Look for rc.name{:=,=,:}value first, because '=' is allowed.
    if (ignoredToken == "rc" ||
        ignoredToken.substr (0, 3) == "rc.")
    {
      if (_eos - _cursor > 1 &&
          (_text[_cursor] == ':' ||
           _text[_cursor] == '='))
      {
        _cursor++;

        if (isString     (ignoredToken, ignoredType, "'\"")  ||
            isContiguous (ignoredToken, ignoredType))
        {
          token = _text.substr (marker, _cursor - marker);
          type = Lexer::Type::pair;
          return true;
        }
      }
    }

    if (_eos - _cursor >= 1    &&
        (_text[_cursor] == ':' ||
         _text[_cursor] == '='))
    {
      _cursor++;

      if (isString     (ignoredToken, ignoredType, "'\"") ||
          isContiguous (ignoredToken, ignoredType)        ||
          _eos == _cursor                                 ||
          _text[_cursor] == ' ')
      {
        token = _text.substr (marker, _cursor - marker);
        type = Lexer::Type::pair;
        return true;
      }
    }
  }

  _cursor = marker;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::set
//   a single number:      1
//   a list of numbers:    1,3,5
//   a list of numbers:    1 3 5
//   a range:              5-10
//   or a combination:     1,3,5-10 12
//
//   <id> [ - <id> ] [ , <id> [ - <id> ] ] ...
bool Lexer::isSet (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;
  bool moreThanJustANumber = false;

  if (isDigit (_text[marker]))
  {
    ++marker;
    while (isDigit (_text[marker]))
      utf8_next_char (_text, marker);

    if (_text[marker] == '-')
    {
      ++marker;
      while (isDigit (_text[marker]))
      {
        utf8_next_char (_text, marker);
        moreThanJustANumber = true;
      }
    }

    while (_text[marker] == ',')
    {
      ++marker;
      while (isDigit (_text[marker]))
        utf8_next_char (_text, marker);

      moreThanJustANumber = true;

      if (_text[marker] == '-')
      {
        ++marker;
        while (isDigit (_text[marker]))
          utf8_next_char (_text, marker);
      }
    }

    if (moreThanJustANumber)
    {
      token = _text.substr (_cursor, marker - _cursor);
      type = Lexer::Type::set;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::tag
//   ^ | '(' | ')' | <isWhiteSpace>
//     [ +|- ] <isIdentifierStart> [ <isIdentifierNext> ]*
bool Lexer::isTag (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  // Lookbehind: Assert ^ or preceded by whitespace, (, or ).
  if (marker > 0                         &&
      ! isWhitespace (_text[marker - 1]) &&
      _text[marker - 1] != '('           &&
      _text[marker - 1] != ')')
    return false;

  if (_text[marker] == '+' ||
      _text[marker] == '-')
  {
    ++marker;

    if (isIdentifierStart (_text[marker]))
    {
      utf8_next_char (_text, marker);

      while (isIdentifierNext (_text[marker]))
          utf8_next_char (_text, marker);

      token = _text.substr (_cursor, marker - _cursor);
      type = Lexer::Type::tag;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::path
//   ( / <non-slash, non-whitespace> )+
bool Lexer::isPath (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;
  int slashCount = 0;

  while (1)
  {
    if (_text[marker] == '/')
    {
      ++marker;
      ++slashCount;
    }
    else
      break;

    if (_text[marker] &&
        ! isWhitespace (_text[marker]) &&
        _text[marker] != '/')
    {
      utf8_next_char (_text, marker);
      while (_text[marker] &&
             ! isWhitespace (_text[marker]) &&
             _text[marker] != '/')
        utf8_next_char (_text, marker);
    }
    else
      break;
  }

  if (marker > _cursor &&
      slashCount > 3)
  {
    type = Lexer::Type::path;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::substitution
//   / <unquoted-string> / <unquoted-string> / [g]  <EOS> | <isWhiteSpace>
bool Lexer::isSubstitution (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  std::string extractedToken;
  Lexer::Type extractedType;
  if (isString (extractedToken, extractedType, "/"))
  {
    --_cursor;  // Step back over the '/'.

    if (isString (extractedToken, extractedType, "/"))
    {
      if (_text[_cursor] == 'g')
        ++_cursor;

      // Lookahread: <EOS> | <isWhitespace>
      if (_text[_cursor] == '\0' ||
          isWhitespace (_text[_cursor]))
      {
        token = _text.substr (marker, _cursor - marker);
        type = Lexer::Type::substitution;
        return true;
      }
    }
  }

  _cursor = marker;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::pattern
//   / <unquoted-string> /  <EOS> | <isWhiteSpace>
bool Lexer::isPattern (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  std::string extractedToken;
  Lexer::Type extractedType;
  if (isString (extractedToken, extractedType, "/") &&
      (_text[_cursor] == '\0' ||
       isWhitespace (_text[_cursor])))
  {
    token = _text.substr (marker, _cursor - marker);
    type = Lexer::Type::pattern;
    return true;
  }

  _cursor = marker;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::op
//   _hastag_ | _notag | _neg_ | _pos_ |
//   <isTripleCharOperator> |
//   <isDoubleCharOperator> |
//   <isSingleCharOperator> |
bool Lexer::isOperator (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  if (_eos - marker >= 8 && _text.substr (marker, 8) == "_hastag_")
  {
    marker += 8;
    type = Lexer::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 7 && _text.substr (marker, 7) == "_notag_")
  {
    marker += 7;
    type = Lexer::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 5 && _text.substr (marker, 5) == "_neg_")
  {
    marker += 5;
    type = Lexer::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 5 && _text.substr (marker, 5) == "_pos_")
  {
    marker += 5;
    type = Lexer::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 3 &&
      isTripleCharOperator (_text[marker], _text[marker + 1], _text[marker + 2], _text[marker + 3]))
  {
    marker += 3;
    type = Lexer::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 2 &&
      isDoubleCharOperator (_text[marker], _text[marker + 1], _text[marker + 2]))
  {
    marker += 2;
    type = Lexer::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (isSingleCharOperator (_text[marker]))
  {
    token = _text[marker];
    type = Lexer::Type::op;
    _cursor = ++marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::dom
//   [ <isUUID> | <isDigit>+ . ] <isIdentifier> [ . <isIdentifier> ]*
bool Lexer::isDOM (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  std::string extractedToken;
  Lexer::Type extractedType;
  if (isUUID (extractedToken, extractedType))
  {
    if (_text[_cursor] == '.')
      ++_cursor;
    else
    {
      _cursor = marker;
      return false;
    }
  }
  else
  {
    if (isDigit (_text[_cursor]))
    {
      ++_cursor;
      while (isDigit (_text[_cursor]))
        ++_cursor;

      if (_text[_cursor] == '.')
        ++_cursor;
      else
      {
        _cursor = marker;
        return false;
      }
    }
  }

  if (! isOperator (extractedToken, extractedType) &&
      isIdentifier (extractedToken, extractedType))
  {
    while (1)
    {
      if (_text[_cursor] == '.')
        ++_cursor;
      else
        break;

      if (isOperator (extractedToken, extractedType) ||
          ! isIdentifier (extractedToken, extractedType))
      {
        _cursor = marker;
        return false;
      }
    }

    type = Lexer::Type::dom;
    token = _text.substr (marker, _cursor - marker);
    return true;
  }

  _cursor = marker;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::identifier
//   <isIdentifierStart> [ <isIdentifierNext> ]*
bool Lexer::isIdentifier (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  if (isIdentifierStart (_text[marker]))
  {
    utf8_next_char (_text, marker);

    while (isIdentifierNext (_text[marker]))
      utf8_next_char (_text, marker);

    token = _text.substr (_cursor, marker - _cursor);
    type = Lexer::Type::identifier;
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::word
//   [^\s]+
bool Lexer::isWord (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  while (_text[marker]                  &&
         ! isWhitespace (_text[marker]) &&
         ! isSingleCharOperator (_text[marker]))
    utf8_next_char (_text, marker);

  if (marker > _cursor)
  {
    token = _text.substr (_cursor, marker - _cursor);
    type = Lexer::Type::word;
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer::Type::word
//   [^\s]+
bool Lexer::isContiguous (std::string& token, Lexer::Type& type)
{
  std::size_t marker = _cursor;

  while (_text[marker]                  &&
         ! isWhitespace (_text[marker]) &&
         _text[marker] != '('           &&
         _text[marker] != ')')
    utf8_next_char (_text, marker);

  if (marker > _cursor)
  {
    token = _text.substr (_cursor, marker - _cursor);
    type = Lexer::Type::word;
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string Lexer::typeToString (Lexer::Type type)
{
       if (type == Lexer::Type::string)       return std::string ("\033[38;5;7m\033[48;5;3m")    + "string"       + "\033[0m";
  else if (type == Lexer::Type::uuid)         return std::string ("\033[38;5;7m\033[48;5;10m")   + "uuid"         + "\033[0m";
  else if (type == Lexer::Type::hex)          return std::string ("\033[38;5;7m\033[48;5;14m")   + "hex"          + "\033[0m";
  else if (type == Lexer::Type::number)       return std::string ("\033[38;5;7m\033[48;5;6m")    + "number"       + "\033[0m";
  else if (type == Lexer::Type::separator)    return std::string ("\033[38;5;7m\033[48;5;4m")    + "separator"    + "\033[0m";
  else if (type == Lexer::Type::url)          return std::string ("\033[38;5;7m\033[48;5;4m")    + "url"          + "\033[0m";
  else if (type == Lexer::Type::pair)         return std::string ("\033[38;5;7m\033[48;5;1m")    + "pair"         + "\033[0m";
  else if (type == Lexer::Type::set)          return std::string ("\033[38;5;15m\033[48;5;208m") + "set"          + "\033[0m";
  else if (type == Lexer::Type::tag)          return std::string ("\033[37;45m")                 + "tag"          + "\033[0m";
  else if (type == Lexer::Type::path)         return std::string ("\033[37;102m")                + "path"         + "\033[0m";
  else if (type == Lexer::Type::substitution) return std::string ("\033[37;102m")                + "substitution" + "\033[0m";
  else if (type == Lexer::Type::pattern)      return std::string ("\033[37;42m")                 + "pattern"      + "\033[0m";
  else if (type == Lexer::Type::op)           return std::string ("\033[38;5;7m\033[48;5;203m")  + "op"           + "\033[0m";
  else if (type == Lexer::Type::dom)          return std::string ("\033[38;5;15m\033[48;5;244m") + "dom"          + "\033[0m";
  else if (type == Lexer::Type::identifier)   return std::string ("\033[38;5;15m\033[48;5;244m") + "identifier"   + "\033[0m";
  else if (type == Lexer::Type::word)         return std::string ("\033[38;5;15m\033[48;5;236m") + "word"         + "\033[0m";
  else if (type == Lexer::Type::date)         return std::string ("\033[38;5;15m\033[48;5;34m")  + "date"         + "\033[0m";
  else if (type == Lexer::Type::duration)     return std::string ("\033[38;5;15m\033[48;5;34m")  + "duration"     + "\033[0m";
  else                                        return std::string ("\033[37;41m")                 + "unknown"      + "\033[0m";
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::isAllDigits (const std::string& text)
{
  return text.find_first_not_of ("0123456789") == std::string::npos;
}

////////////////////////////////////////////////////////////////////////////////
// Not escape-proof.
bool Lexer::isOneWord (const std::string& text)
{
  std::string::size_type i = 0;
  int character;
  while ((character = utf8_next_char (text, i)))
    if (Lexer::isWhitespace (character))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Full implementation of a quoted word.  Includes:
//   '\''
//   '"'
//   "'"
//   "\""
//   'one two'
// Result includes the quotes.
bool Lexer::readWord (
  const std::string& text,
  const std::string& quotes,
  std::string::size_type& cursor,
  std::string& word)
{
  if (quotes.find (text[cursor]) == std::string::npos)
    return false;

  std::string::size_type eos = text.length ();
  int quote = text[cursor++];
  word = quote;

  int c;
  while ((c = text[cursor]))
  {
    // Quoted word ends on a quote.
    if (quote && quote == c)
    {
      word += utf8_character (utf8_next_char (text, cursor));
      break;
    }

    // Unicode U+XXXX or \uXXXX codepoint.
    else if (eos - cursor >= 6 &&
             ((text[cursor + 0] == 'U'  && text[cursor + 1] == '+') ||
              (text[cursor + 0] == '\\' && text[cursor + 1] == 'u')) &&
             isHexDigit (text[cursor + 2]) &&
             isHexDigit (text[cursor + 3]) &&
             isHexDigit (text[cursor + 4]) &&
             isHexDigit (text[cursor + 5]))
    {
      word += utf8_character (
                hexToInt (
                  text[cursor + 2],
                  text[cursor + 3],
                  text[cursor + 4],
                  text[cursor + 5]));
      cursor += 6;
    }

    // An escaped thing.
    else if (c == '\\')
    {
      c = text[++cursor];

      switch (c)
      {
      case '"':  word += (char) 0x22; ++cursor; break;
      case '\'': word += (char) 0x27; ++cursor; break;
      case '\\': word += (char) 0x5C; ++cursor; break;
      case 'b':  word += (char) 0x08; ++cursor; break;
      case 'f':  word += (char) 0x0C; ++cursor; break;
      case 'n':  word += (char) 0x0A; ++cursor; break;
      case 'r':  word += (char) 0x0D; ++cursor; break;
      case 't':  word += (char) 0x09; ++cursor; break;
      case 'v':  word += (char) 0x0B; ++cursor; break;

      // This pass-through default case means that anything can be escaped
      // harmlessly. In particular 'quote' is included, if it not one of the
      // above characters.
      default:   word += (char) c;    ++cursor; break;
      }
    }

    // Ordinary character.
    else
      word += utf8_character (utf8_next_char (text, cursor));
  }

  return word.length () > 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// Full implementation of an unquoted word.  Includes:
//   one\ two
//   abcU+0020def
//   abc\u0020def
//   a\tb
bool Lexer::readWord (
  const std::string& text,
  std::string::size_type& cursor,
  std::string& word)
{
  std::string::size_type eos = text.length ();

  word = "";
  int c;
  while ((c = text[cursor]))
  {
    // Unquoted word ends on white space.
    if (Lexer::isWhitespace (c))
      break;

    // Unicode U+XXXX or \uXXXX codepoint.
    else if (eos - cursor >= 6 &&
             ((text[cursor + 0] == 'U'  && text[cursor + 1] == '+') ||
              (text[cursor + 0] == '\\' && text[cursor + 1] == 'u')) &&
             isHexDigit (text[cursor + 2]) &&
             isHexDigit (text[cursor + 3]) &&
             isHexDigit (text[cursor + 4]) &&
             isHexDigit (text[cursor + 5]))
    {
      word += utf8_character (
                hexToInt (
                  text[cursor + 2],
                  text[cursor + 3],
                  text[cursor + 4],
                  text[cursor + 5]));
      cursor += 6;
    }

    // An escaped thing.
    else if (c == '\\')
    {
      c = text[++cursor];

      switch (c)
      {
      case '"':  word += (char) 0x22; ++cursor; break;
      case '\'': word += (char) 0x27; ++cursor; break;
      case '\\': word += (char) 0x5C; ++cursor; break;
      case 'b':  word += (char) 0x08; ++cursor; break;
      case 'f':  word += (char) 0x0C; ++cursor; break;
      case 'n':  word += (char) 0x0A; ++cursor; break;
      case 'r':  word += (char) 0x0D; ++cursor; break;
      case 't':  word += (char) 0x09; ++cursor; break;
      case 'v':  word += (char) 0x0B; ++cursor; break;

      // This pass-through default case means that anything can be escaped
      // harmlessly. In particular 'quote' is included, if it not one of the
      // above characters.
      default:   word += (char) c;    ++cursor; break;
      }
    }

    // Ordinary character.
    else
      word += utf8_character (utf8_next_char (text, cursor));
  }

  return word.length () > 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// <name> [. <modifier>] <: | = | :: | :=> [<value>]
bool Lexer::decomposePair (
  const std::string& text,
  std::string& name,
  std::string& modifier,
  std::string& separator,
  std::string& value)
{
  // Look for the required elements.
  std::string::size_type dot       = text.find ('.');
  std::string::size_type sep_defer = text.find ("::");
  std::string::size_type sep_eval  = text.find (":=");
  std::string::size_type sep_colon = text.find (':');
  std::string::size_type sep_equal = text.find ('=');

  // Determine which separator is dominant, which would be the first one seen,
  // taking into consideration the overlapping : characters.
  std::string::size_type sep     = std::string::npos;
  std::string::size_type sep_end = std::string::npos;
  if (sep_defer != std::string::npos &&
      (sep_eval  == std::string::npos || sep_defer <= sep_eval)  &&
      (sep_colon == std::string::npos || sep_defer <= sep_colon) &&
      (sep_equal == std::string::npos || sep_defer <= sep_equal))
  {
    sep     = sep_defer;
    sep_end = sep_defer + 2;
  }
  else if (sep_eval != std::string::npos &&
           (sep_defer == std::string::npos || sep_eval <= sep_defer) &&
           (sep_colon == std::string::npos || sep_eval <= sep_colon) &&
           (sep_equal == std::string::npos || sep_eval <= sep_equal))
  {
    sep     = sep_eval;
    sep_end = sep_eval + 2;
  }
  else if (sep_colon != std::string::npos &&
           (sep_defer == std::string::npos || sep_colon <= sep_defer) &&
           (sep_eval  == std::string::npos || sep_colon <= sep_eval)  &&
           (sep_equal == std::string::npos || sep_colon <= sep_equal))
  {
    sep     = sep_colon;
    sep_end = sep_colon + 1;
  }
  else if (sep_equal != std::string::npos &&
           (sep_defer == std::string::npos || sep_equal <= sep_defer) &&
           (sep_eval  == std::string::npos || sep_equal <= sep_eval)  &&
           (sep_colon == std::string::npos || sep_equal <= sep_colon))
  {
    sep     = sep_equal;
    sep_end = sep_equal + 1;
  }

  // If sep is known, all is well.
  if (sep != std::string::npos)
  {
    // Now the only unknown is whethere there is a modifier.
    if (dot != std::string::npos && dot < sep)
    {
      name     = text.substr (0, dot);
      modifier = text.substr (dot + 1, sep - dot - 1);
    }
    else
    {
      name     = text.substr (0, sep);
      modifier = "";
    }

    separator = text.substr (sep, sep_end - sep);
    value     = text.substr (sep_end);

    // An empty name is an error.
    if (name.length ())
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
