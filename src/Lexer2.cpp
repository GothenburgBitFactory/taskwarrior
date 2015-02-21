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
#include <Lexer2.h>
#include <utf8.h>

static const std::string uuid_pattern = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
static const int uuid_min_length = 8;

////////////////////////////////////////////////////////////////////////////////
Lexer2::Lexer2 (const std::string& text)
: _text (text)
, _cursor (0)
, _eos (text.size ())
{
}

////////////////////////////////////////////////////////////////////////////////
Lexer2::~Lexer2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
// When a Lexer2 object is constructed with a string, this method walks through
// the stream of low-level tokens.
bool Lexer2::token (std::string& token, Lexer2::Type& type)
{
  // Eat white space.
  while (isWhitespace (_text[_cursor]))
    utf8_next_char (_text, _cursor);

  // Terminate at EOS.
  if (isEOS ())
    return false;

  // The sequence is specific, and must follow these rules:
  // - date < duration < uuid < identifier
  // - url < pair < identifier
  // - hex < number
  // - separator < tag < operator
  // - path < substitution < pattern
  // - word last
  if (isString       (token, type, '\'') ||
      isString       (token, type, '"')  ||
      isDate         (token, type)       ||
      isDuration     (token, type)       ||
      isUUID         (token, type)       ||
      isHexNumber    (token, type)       ||
      isNumber       (token, type)       ||
      isSeparator    (token, type)       ||
      isList         (token, type)       ||
      isURL          (token, type)       ||
      isPair         (token, type)       ||
      isTag          (token, type)       ||
      isPath         (token, type)       ||
      isSubstitution (token, type)       ||
      isPattern      (token, type)       ||
      isOperator     (token, type)       ||
      isIdentifier   (token, type)       ||
      isWord         (token, type))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// This static method tokenizes the input and provides a vector of token/type
// results from a high-level lex.
std::vector <std::pair <std::string, Lexer2::Type>> Lexer2::tokens (
  const std::string& text)
{
  std::vector <std::pair <std::string, Lexer2::Type>> all;
  std::string token;
  Lexer2::Type type;
  Lexer2 l (text);
  while (l.token (token, type))
    all.push_back (std::pair <std::string, Lexer2::Type> (token, type));

  return all;
}

////////////////////////////////////////////////////////////////////////////////
// This static method tokenizes the input, but discards the type information.
std::vector <std::string> Lexer2::split (const std::string& text)
{
  std::vector <std::string> all;
  std::string token;
  Lexer2::Type ignored;
  Lexer2 l (text);
  while (l.token (token, ignored))
    all.push_back (token);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
// No L10N - these are for internal purposes.
const std::string Lexer2::typeName (const Lexer2::Type& type)
{
  switch (type)
  {
  case Lexer2::Type::uuid:         return "uuid";
  case Lexer2::Type::number:       return "number";
  case Lexer2::Type::hex:          return "hex";
  case Lexer2::Type::string:       return "string";
  case Lexer2::Type::list:         return "list";
  case Lexer2::Type::url:          return "url";
  case Lexer2::Type::pair:         return "pair";
  case Lexer2::Type::separator:    return "separator";
  case Lexer2::Type::tag:          return "tag";
  case Lexer2::Type::path:         return "path";
  case Lexer2::Type::substitution: return "substitution";
  case Lexer2::Type::pattern:      return "pattern";
  case Lexer2::Type::op:           return "op";
  case Lexer2::Type::identifier:   return "identifier";
  case Lexer2::Type::word:         return "word";
  case Lexer2::Type::date:         return "date";
  case Lexer2::Type::duration:     return "duration";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Complete Unicode whitespace list.
//
// http://en.wikipedia.org/wiki/Whitespace_character
// Updated 2013-11-18
// Static
bool Lexer2::isWhitespace (int c)
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
// Digits 0-9.
bool Lexer2::isDigit (int c)
{
  return c >= 0x30 && c <= 0x39;
}

////////////////////////////////////////////////////////////////////////////////
// Digits 0-9 a-f A-F.
bool Lexer2::isHexDigit (int c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer2::isIdentifierStart (int c)
{
  return c                          &&  // Include null character check.
         ! isWhitespace         (c) &&
         ! isDigit              (c) &&
         ! isSingleCharOperator (c) &&
         ! isPunctuation        (c);
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer2::isIdentifierNext (int c)
{
  return c                          &&  // Include null character check.
         c != ':'                   &&  // Used in isPair.
         ! isWhitespace         (c) &&
         ! isSingleCharOperator (c);
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer2::isSingleCharOperator (int c)
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
bool Lexer2::isDoubleCharOperator (int c0, int c1, int c2)
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
bool Lexer2::isTripleCharOperator (int c0, int c1, int c2, int c3)
{
  return (c0 == 'a' && c1 == 'n' && c2 == 'd' && isBoundary (c2, c3)) ||
         (c0 == 'x' && c1 == 'o' && c2 == 'r' && isBoundary (c2, c3)) ||
         (c0 == '!' && c1 == '=' && c2 == '=');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer2::isBoundary (int left, int right)
{
  // XOR
  if (isalpha (left)       != isalpha (right))       return true;
  if (isDigit (left)       != isDigit (right))       return true;
  if (isWhitespace (left)  != isWhitespace (right))  return true;

  // OR
  if (isPunctuation (left) || isPunctuation (right)) return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer2::isPunctuation (int c)
{
  return c != '@' &&
         ispunct (c);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer2::dequote (std::string& input)
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
bool Lexer2::isEOS () const
{
  return _cursor >= _eos;
}

////////////////////////////////////////////////////////////////////////////////
// Converts '0'     -> 0
//          '9'     -> 9
//          'a'/'A' -> 10
//          'f'/'F' -> 15
int Lexer2::hexToInt (int c) const
{
       if (c >= '0' && c <= '9') return (c - '0');
  else if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
  else                           return (c - 'A' + 10);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer2::hexToInt (int c0, int c1) const
{
  return (hexToInt (c0) << 4) + hexToInt (c1);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer2::hexToInt (int c0, int c1, int c2, int c3) const
{
  return (hexToInt (c0) << 12) +
         (hexToInt (c1) << 8)  +
         (hexToInt (c2) << 4)  +
          hexToInt (c3);
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::string
//   '|"
//   [ U+XXXX | \uXXXX | \" | \' | \\ | \/ | \b | \f | \n | \r | \t | . ]
//   '|"
bool Lexer2::isString (std::string& token, Lexer2::Type& type, int quote)
{
  std::size_t marker = _cursor;

  if (_text[marker] == quote)
  {
    ++marker;
    token = "";

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
      ++marker;
      type = Lexer2::Type::string;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::date
//   
bool Lexer2::isDate (std::string& token, Lexer2::Type& type)
{
#if 0
  // Try an ISO date parse.
  if (isoEnabled)
  {
    std::string::size_type iso_i = 0;
    std::string iso_result;
    ISO8601d iso;
    iso.ambiguity (_ambiguity);
    if (iso.parse (_input.substr (_shift_counter), iso_i))
    {
      result = _input.substr (_shift_counter, iso_i);
      while (iso_i--) shift ();
      return true;
    }
  }

  // Try a legacy rc.dateformat parse here.
  if (Lexer::dateFormat != "")
  {
    try
    {
      std::string::size_type legacy_i = 0;
      Date legacyDate (_input.substr (_shift_counter), legacy_i, Lexer::dateFormat, false, false);
      result = _input.substr (_shift_counter, legacy_i);
      while (legacy_i--) shift ();
      return true;
    }

    catch (...) { /* Never mind. */ }
  }
#endif

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::duration
//   
bool Lexer2::isDuration (std::string& token, Lexer2::Type& type)
{
#if 0
  std::string::size_type iso_i = 0;
  std::string iso_result;
  ISO8601p iso;
  if (iso.parse (_input.substr (_shift_counter), iso_i))
  {
    result = _input.substr (_shift_counter, iso_i);
    while (iso_i--) shift ();
    return true;
  }

  std::string::size_type dur_i = 0;
  std::string dur_result;
  Duration dur;
  if (dur.parse (_input.substr (_shift_counter), dur_i))
  {
    result = _input.substr (_shift_counter, dur_i);
    while (dur_i--) shift ();
    return true;
  }
#endif

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::uuid
//   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
//   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXX
//   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXX
//   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXX
//   ...
//   XXXXXXXX-XX
//   XXXXXXXX-X
//   XXXXXXXX-
//   XXXXXXXX
bool Lexer2::isUUID (std::string& token, Lexer2::Type& type)
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
    token = _text.substr (_cursor, i + 1);
    type = Lexer2::Type::uuid;
    _cursor += i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::hex
//   0xX+
bool Lexer2::isHexNumber (std::string& token, Lexer2::Type& type)
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
      type = Lexer2::Type::hex;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::number
//   \d+
//   [ . \d+ ]
//   [ e|E [ +|- ] \d+ ]
bool Lexer2::isNumber (std::string& token, Lexer2::Type& type)
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
      }
    }

    token = _text.substr (_cursor, marker - _cursor);
    type = Lexer2::Type::number;
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::separator
//   --
bool Lexer2::isSeparator (std::string& token, Lexer2::Type& type)
{
  if (_eos - _cursor >= 2 &&
      _text[_cursor] == '-' &&
      _text[_cursor + 1] == '-')
  {
    _cursor += 2;
    type = Lexer2::Type::separator;
    token = "--";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::list
//   ,
bool Lexer2::isList (std::string& token, Lexer2::Type& type)
{
  if (_eos - _cursor > 1 &&
      _text[_cursor] == ',')
  {
    ++_cursor;
    type = Lexer2::Type::list;
    token = ",";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::url
//   http [s] :// ...
bool Lexer2::isURL (std::string& token, Lexer2::Type& type)
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
      type = Lexer2::Type::url;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::pair
//   <identifier> : [ <string> | <word> ]
bool Lexer2::isPair (std::string& token, Lexer2::Type& type)
{
  std::size_t marker = _cursor;

  std::string ignoredToken;
  Lexer2::Type ignoredType;
  if (isIdentifier (ignoredToken, ignoredType))
  {
    if (_eos - _cursor > 1 &&
        (_text[_cursor] == ':' || _text[_cursor] == '='))
    {
      _cursor++;

      if (isString (ignoredToken, ignoredType, '\'') ||
          isString (ignoredToken, ignoredType, '"')  ||
          isWord   (ignoredToken, ignoredType))
      {
        token = _text.substr (marker, _cursor - marker);
        type = Lexer2::Type::pair;
        return true;
      }
    }
  }

  _cursor = marker;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::tag
//   [ +|- ] <isIdentifierStart> [ <isIdentifierNext> ]*
bool Lexer2::isTag (std::string& token, Lexer2::Type& type)
{
  std::size_t marker = _cursor;

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
      type = Lexer2::Type::tag;
      _cursor = marker;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::path
//   ( / <non-slash, non-whitespace> )+
bool Lexer2::isPath (std::string& token, Lexer2::Type& type)
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

    if (! isWhitespace (_text[marker]) &&
        _text[marker] != '/')
    {
      utf8_next_char (_text, marker);
      while (! isWhitespace (_text[marker]) &&
             _text[marker] != '/')
        utf8_next_char (_text, marker);
    }
    else
      break;
  }

  if (marker > _cursor &&
      slashCount > 3)
  {
    type = Lexer2::Type::path;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::substitution
//   / <unquoted-string> / <unquoted-string> / [g]
bool Lexer2::isSubstitution (std::string& token, Lexer2::Type& type)
{
  std::size_t marker = _cursor;

  std::string extractedToken;
  Lexer2::Type extractedType;
  if (isString (extractedToken, extractedType, '/'))
  {
    --_cursor;  // Step back over the '/'.

    if (isString (extractedToken, extractedType, '/'))
    {
      if (_text[_cursor] == 'g')
        ++_cursor;

      if (isWhitespace (_text[_cursor]))
      {
        token = _text.substr (marker, _cursor - marker);
        type = Lexer2::Type::substitution;
        return true;
      }
    }
  }

  _cursor = marker;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::pattern
//   / <unquoted-string> /
bool Lexer2::isPattern (std::string& token, Lexer2::Type& type)
{
  std::size_t marker = _cursor;

  std::string extractedToken;
  Lexer2::Type extractedType;
  if (isString (extractedToken, extractedType, '/') &&
      isWhitespace (_text[_cursor]))
  {
    token = _text.substr (marker, _cursor - marker);
    type = Lexer2::Type::pattern;
    return true;
  }

  _cursor = marker;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::op
//   _hastag_ | _notag | _neg_ | _pos_ |
//   <isTripleCharOperator> |
//   <isDoubleCharOperator> |
//   <isSingleCharOperator> |
bool Lexer2::isOperator (std::string& token, Lexer2::Type& type)
{
  std::size_t marker = _cursor;

  if (_eos - marker >= 8 && _text.substr (marker, 8) == "_hastag_")
  {
    marker += 8;
    type = Lexer2::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 7 && _text.substr (marker, 7) == "_notag_")
  {
    marker += 7;
    type = Lexer2::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 5 && _text.substr (marker, 5) == "_neg_")
  {
    marker += 5;
    type = Lexer2::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 5 && _text.substr (marker, 5) == "_pos_")
  {
    marker += 5;
    type = Lexer2::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 4 &&
      isTripleCharOperator (_text[marker], _text[marker + 1], _text[marker + 2], _text[marker + 3]))
  {
    marker += 3;
    type = Lexer2::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (_eos - marker >= 2 &&
      isDoubleCharOperator (_text[marker], _text[marker + 1], _text[marker + 2]))
  {
    marker += 2;
    type = Lexer2::Type::op;
    token = _text.substr (_cursor, marker - _cursor);
    _cursor = marker;
    return true;
  }

  else if (isSingleCharOperator (_text[marker]))
  {
    token = _text[marker];
    type = Lexer2::Type::op;
    _cursor = ++marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::identifier
//   <isIdentifierStart> [ <isIdentifierNext> ]*
bool Lexer2::isIdentifier (std::string& token, Lexer2::Type& type)
{
  std::size_t marker = _cursor;

  if (isIdentifierStart (_text[marker]))
  {
    utf8_next_char (_text, marker);

    while (isIdentifierNext (_text[marker]))
        utf8_next_char (_text, marker);

    token = _text.substr (_cursor, marker - _cursor);
    type = Lexer2::Type::identifier;
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Lexer2::Type::word
//   [^\s]+
bool Lexer2::isWord (std::string& token, Lexer2::Type& type)
{
  std::size_t marker = _cursor;

  while (! isWhitespace (_text[marker]))
    utf8_next_char (_text, marker);

  if (marker > _cursor)
  {
    token = _text.substr (_cursor, marker - _cursor);
    type = Lexer2::Type::word;
    _cursor = marker;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string Lexer2::typeToString (Lexer2::Type type)
{
       if (type == Lexer2::Type::string)       return std::string ("\033[38;5;7m\033[48;5;3m")    + "string"       + "\033[0m";
  else if (type == Lexer2::Type::uuid)         return std::string ("\033[38;5;7m\033[48;5;10m")   + "uuid"         + "\033[0m";
  else if (type == Lexer2::Type::hex)          return std::string ("\033[38;5;7m\033[48;5;14m")   + "hex"          + "\033[0m";
  else if (type == Lexer2::Type::number)       return std::string ("\033[38;5;7m\033[48;5;6m")    + "number"       + "\033[0m";
  else if (type == Lexer2::Type::separator)    return std::string ("\033[38;5;7m\033[48;5;4m")    + "separator"    + "\033[0m";
  else if (type == Lexer2::Type::list)         return std::string ("\033[38;5;7m\033[48;5;4m")    + "list"         + "\033[0m";
  else if (type == Lexer2::Type::url)          return std::string ("\033[38;5;7m\033[48;5;4m")    + "url"          + "\033[0m";
  else if (type == Lexer2::Type::pair)         return std::string ("\033[38;5;7m\033[48;5;1m")    + "pair"         + "\033[0m";
  else if (type == Lexer2::Type::tag)          return std::string ("\033[37;45m")                 + "tag"          + "\033[0m";
  else if (type == Lexer2::Type::path)         return std::string ("\033[37;102m")                + "path"         + "\033[0m";
  else if (type == Lexer2::Type::substitution) return std::string ("\033[37;102m")                + "substitution" + "\033[0m";
  else if (type == Lexer2::Type::pattern)      return std::string ("\033[37;42m")                 + "pattern"      + "\033[0m";
  else if (type == Lexer2::Type::op)           return std::string ("\033[38;5;7m\033[48;5;203m")  + "op"           + "\033[0m";
  else if (type == Lexer2::Type::identifier)   return std::string ("\033[38;5;15m\033[48;5;244m") + "identifier"   + "\033[0m";
  else if (type == Lexer2::Type::word)         return std::string ("\033[38;5;15m\033[48;5;236m") + "word"         + "\033[0m";
  else if (type == Lexer2::Type::date)         return std::string ("\033[38;5;15m\033[48;5;34")   + "date"         + "\033[0m";
  else if (type == Lexer2::Type::duration)     return std::string ("\033[38;5;15m\033[48;5;34")   + "duration"     + "\033[0m";
  else                                         return std::string ("\033[37;41m")                 + "unknown"      + "\033[0m";
}

////////////////////////////////////////////////////////////////////////////////
