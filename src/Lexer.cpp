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
#include <utf8.h>
#include <ISO8601.h>
#include <Date.h>
#include <Duration.h>
#include <Lexer.h>
#include <i18n.h>

std::string Lexer::dateFormat = "";

////////////////////////////////////////////////////////////////////////////////
Lexer::Lexer (const std::string& input)
: _input (input)
, _i (0)
, _shift_counter (0)
, _n0 (32)
, _n1 (32)
, _n2 (32)
, _n3 (32)
, _boundary01 (false)
, _boundary12 (false)
, _boundary23 (false)
, _ambiguity (true)
{
  // Read 4 chars in preparation.  Even if there are < 4.  Take a deep breath.
  shift ();
  shift ();
  shift ();
  shift ();

  // Reset because the four shifts above do not represent advancement into the
  // _input. All subsequents shiftѕ do though.
  _shift_counter = 0;
}

////////////////////////////////////////////////////////////////////////////////
Lexer::~Lexer ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Walk the input string, looking for transitions.
bool Lexer::token (std::string& result, Type& type)
{
  // Start with nothing.
  result = "";

  // Different types of matching quote:  ', ".
  int quote = 0;

  type = typeNone;
  while (_n0)
  {
    switch (type)
    {
    case typeNone:
      if (is_ws (_n0))
        shift ();
      else if (_n0 == '"' || _n0 == '\'')
      {
        type = typeString;
        quote = _n0;
        result += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == '0' &&
               _n1 == 'x' &&
               is_hex_digit (_n2))
      {
        type = typeHex;
        result += utf8_character (_n0);
        shift ();
        result += utf8_character (_n0);
        shift ();
        result += utf8_character (_n0);
        shift ();
      }
      else if (is_dec_digit (_n0))
      {
        // Speculatively try a date and duration parse.  Longest wins.
        if (is_date (result))
        {
          type = typeDate;
          return true;
        }

        if (is_duration (result))
        {
          type = typeDuration;
          return true;
        }

        type = typeNumber;
        result += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == '.' && is_dec_digit (_n1))
      {
        type = typeDecimal;
        result += utf8_character (_n0);
        shift ();
      }
      else if ((_n0 == '+' || _n0 == '-') && is_ident_start (_n1))
      {
        type = typeTag;
        result += utf8_character (_n0);
        shift ();
      }
      else if (is_triple_op (_n0, _n1, _n2))
      {
        type = typeOperator;
        result += utf8_character (_n0);
        shift ();
        result += utf8_character (_n0);
        shift ();
        result += utf8_character (_n0);
        shift ();
        return true;
      }
      else if (is_double_op (_n0, _n1, _n2))
      {
        type = typeOperator;
        result += utf8_character (_n0);
        shift ();
        result += utf8_character (_n0);
        shift ();
        return true;
      }
      else if (is_single_op (_n0))
      {
        type = typeOperator;
        result += utf8_character (_n0);
        shift ();
        return true;
      }
      else if (_n0 == '\\')
      {
        type = typeIdentifierEscape;
        shift ();
      }
      else if (is_ident_start (_n0))
      {
        if (is_date (result))
        {
          type = typeDate;
          return true;
        }

        if (is_duration (result))
        {
          type = typeDuration;
          return true;
        }

        type = typeIdentifier;
        result += utf8_character (_n0);
        shift ();
      }
      else
        throw std::string (STRING_LEX_IMMEDIATE_UNK);
      break;

    case typeString:
      if (_n0 == quote)
      {
        result += utf8_character (_n0);
        shift ();
        quote = 0;
        return true;
      }
      else if (_n0 == '\\')
      {
        type = typeEscape;
        shift ();
      }
      else
      {
        result += utf8_character (_n0);
        shift ();
      }
      break;

    case typeTag:
      if (is_ident_start (_n0))
      {
        result += utf8_character (_n0);
        shift ();
      }
      else
      {
        return true;
      }
      break;

    case typeIdentifier:
      if (is_ident (_n0))
      {
        result += utf8_character (_n0);
        shift ();
      }
      else
      {
        // typeIdentifier is a catch-all type. Anything word-like becomes an
        // identifier. At this point in the processing, an identifier is found,
        // and can be matched against a list of potential upgrades.
        if (result == "_hastag_" ||
            result == "_notag_"  ||
            result == "_neg_"    ||
            result == "_pos_")
          type = typeOperator;

        return true;
      }
      break;

    case typeIdentifierEscape:
      if (_n0 == 'u')
      {
        type = typeEscapeUnicode;
        shift ();
      }
      else
      {
        type = quote ? typeString : typeIdentifier;
        result += utf8_character (quote);
        result += utf8_character (_n0);
        shift ();
      }
      break;

    case typeEscape:
      if (_n0 == 'x')
      {
        type = typeEscapeHex;
        shift ();
      }
      else if (_n0 == 'u')
      {
        type = typeEscapeUnicode;
        shift ();
      }
      else
      {
        result += '\\';
        result += utf8_character (_n0);
        type = quote ? typeString : typeIdentifier;
        shift ();
      }
      break;

    case typeEscapeHex:
      if (is_hex_digit (_n0) && is_hex_digit (_n1))
      {
        result += utf8_character (hex_to_int (_n0, _n1));
        type = quote ? typeString : typeIdentifier;
        shift ();
        shift ();
      }
      else
      {
        type = quote ? typeString : typeIdentifier;
        shift ();
        quote = 0;
        return true;
      }
      break;

    case typeEscapeUnicode:
      if (is_hex_digit (_n0) &&
          is_hex_digit (_n1) &&
          is_hex_digit (_n2) &&
          is_hex_digit (_n3))
      {
        result += utf8_character (hex_to_int (_n0, _n1, _n2, _n3));
        shift ();
        shift ();
        shift ();
        shift ();
        type = quote ? typeString : typeIdentifier;
      }
      else if (_n0 == quote)
      {
        type = typeString;
        shift ();
        quote = 0;
        return true;
      }
      break;

    case typeNumber:
      if (is_dec_digit (_n0))
      {
        result += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == '.')
      {
        type = typeDecimal;
        result += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == 'e' || _n0 == 'E')
      {
        type = typeExponentIndicator;
        result += utf8_character (_n0);
        shift ();
      }
      else if (is_ident_start (_n0))
      {
        type = typeIdentifier;
        result += utf8_character (_n0);
        shift ();
      }
      else
      {
        return true;
      }
      break;

    case typeDecimal:
      if (is_dec_digit (_n0))
      {
        result += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == 'e' || _n0 == 'E')
      {
        type = typeExponentIndicator;
        result += utf8_character (_n0);
        shift ();
      }
      else if (is_ident_start (_n0))
      {
        type = typeIdentifier;
        result += utf8_character (_n0);
        shift ();
      }
      else
      {
        return true;
      }
      break;

    case typeExponentIndicator:
      if (_n0 == '+' || _n0 == '-')
      {
        result += utf8_character (_n0);
        shift ();
      }
      else if (is_dec_digit (_n0))
      {
        type = typeExponent;
        result += utf8_character (_n0);
        shift ();
      }
      else if (is_ident_start (_n0))
      {
        type = typeIdentifier;
        result += utf8_character (_n0);
        shift ();
      }
      break;

    case typeExponent:
      if (is_dec_digit (_n0) || _n0 == '.')
      {
        result += utf8_character (_n0);
        shift ();
      }
      else
      {
        type = typeDecimal;
        return true;
      }
      break;

    case typeHex:
      if (is_hex_digit (_n0))
      {
        result += utf8_character (_n0);
        shift ();
      }
      else
      {
        return true;
      }
      break;

    default:
      throw std::string (STRING_LEX_TYPE_UNK);
      break;
    }

    // Fence post.
    if (!_n0 && result != "")
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Just like Lexer::token, but no operators, numbers, dates or durations.
bool Lexer::word (std::string& token, Type& type)
{
  // Start with nothing.
  token = "";

  // Different types of matching quote:  ', ".
  int quote = 0;

  type = typeNone;
  while (_n0)
  {
    switch (type)
    {
    case typeNone:
      if (is_ws (_n0))
        shift ();
      else if (_n0 == '"' || _n0 == '\'')
      {
        type = typeString;
        quote = _n0;
        token += utf8_character (_n0);
        shift ();
      }
      else
      {
        type = typeString;
        token += utf8_character (_n0);
        shift ();
      }
      break;

    case typeString:
      if (_n0 == quote)
      {
        token += utf8_character (_n0);
        shift ();
        quote = 0;
        return true;
      }
      else if (_n0 == '\\')
      {
        type = typeEscape;
        shift ();
      }
      else if (! quote && is_ws (_n0))
      {
        shift ();
        return true;
      }
      else
      {
        token += utf8_character (_n0);
        shift ();
      }
      break;

    case typeEscape:
      if (_n0 == 'x')
      {
        type = typeEscapeHex;
        shift ();
      }
      else if (_n0 == 'u')
      {
        type = typeEscapeUnicode;
        shift ();
      }
      else
      {
        token += '\\';
        token += utf8_character (_n0);
        type = typeString;
        shift ();
      }
      break;

    case typeEscapeHex:
      if (is_hex_digit (_n0) && is_hex_digit (_n1))
      {
        token += utf8_character (hex_to_int (_n0, _n1));
        type = typeString;
        shift ();
        shift ();
      }
      else
      {
        type = typeString;
        shift ();
        quote = 0;
        return true;
      }
      break;

    case typeEscapeUnicode:
      if (is_hex_digit (_n0) &&
          is_hex_digit (_n1) &&
          is_hex_digit (_n2) &&
          is_hex_digit (_n3))
      {
        token += utf8_character (hex_to_int (_n0, _n1, _n2, _n3));
        shift ();
        shift ();
        shift ();
        shift ();
        type = typeString;
      }
      else if (_n0 == quote)
      {
        type = typeString;
        shift ();
        quote = 0;
        return true;
      }
      break;

    default:
      throw std::string (STRING_LEX_TYPE_UNK);
      break;
    }

    // Fence post.
    if (!_n0 && token != "")
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::ambiguity (bool value)
{
  _ambiguity = value;
}

////////////////////////////////////////////////////////////////////////////////
// No L10N - these are for internal purposes.
const std::string Lexer::type_name (const Type& type)
{
  switch (type)
  {
  case Lexer::typeNone:              return "None";
  case Lexer::typeString:            return "String";
  case Lexer::typeIdentifier:        return "Identifier";
  case Lexer::typeIdentifierEscape:  return "IdentifierEscape";
  case Lexer::typeNumber:            return "Number";
  case Lexer::typeDecimal:           return "Decimal";
  case Lexer::typeExponentIndicator: return "ExponentIndicator";
  case Lexer::typeExponent:          return "Exponent";
  case Lexer::typeHex:               return "Hex";
  case Lexer::typeOperator:          return "Operator";
  case Lexer::typeEscape:            return "Escape";
  case Lexer::typeEscapeHex:         return "EscapeHex";
  case Lexer::typeEscapeUnicode:     return "EscapeUnicode";
  case Lexer::typeDate:              return "Date";
  case Lexer::typeDuration:          return "Duration";
  case Lexer::typeTag:               return "Tag";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Complete Unicode whitespace list.
//
// http://en.wikipedia.org/wiki/Whitespace_character
// Updated 2013-11-18
bool Lexer::is_ws (int c)
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
bool Lexer::is_ident_start (int c)
{
  return c           &&       // Include null character check.
         ! is_ws (c) &&
         ! is_dec_digit (c) &&
         ! is_single_op (c);
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_ident (int c)
{
  return c           &&       // Include null character check.
         ! is_ws (c) &&
         ! is_single_op (c);
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_single_op (int c)
{
  return c == '+' ||
         c == '-' ||
         c == '*' ||
         c == '/' ||
         c == '(' ||
         c == ')' ||
         c == '<' ||
         c == '>' ||
         c == '^' ||
         c == '!' ||
         c == '%' ||
         c == '=' ||
         c == '~';
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_dec_digit (int c)
{
  return c >= '0' && c <= '9';
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::boundary (int left, int right)
{
  // XOR
  if (isalpha (left) != isalpha (right)) return true;
  if (isdigit (left) != isdigit (right)) return true;
  if (isspace (left) != isspace (right)) return true;

  // OR
  if (ispunct (left)  || ispunct (right))  return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Split 'input' into 'words' on Lexer::is_ws boundaries, observing quotes.
void Lexer::word_split (std::vector <std::string>& words, const std::string& input)
{
  words.clear ();

  std::string word;
  Lexer::Type type;
  Lexer lex (input);
  while (lex.word (word, type))
    words.push_back (word);
}

////////////////////////////////////////////////////////////////////////////////
// Split 'input' into 'tokens'.
void Lexer::token_split (std::vector <std::string>& words, const std::string& input)
{
  words.clear ();

  std::string word;
  Lexer::Type type;
  Lexer lex (input);
  while (lex.token (word, type))
    words.push_back (word);
}

////////////////////////////////////////////////////////////////////////////////
// Split 'input' into 'tokens', preserving type.
void Lexer::token_split (std::vector <std::pair <std::string, Lexer::Type> >& lexemes, const std::string& input)
{
  lexemes.clear ();

  std::string word;
  Lexer::Type type;
  Lexer lex (input);
  while (lex.token (word, type))
    lexemes.push_back (std::pair <std::string, Lexer::Type>(word, type));
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
bool Lexer::is_date (std::string& result)
{
  // Try an ISO date parse.
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

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_duration (std::string& result)
{
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

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_punct (int c) const
{
  if (c == ',' ||
      c == '.')      // Tab
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_num (int c) const
{
  if ((c >= '0' && c <= '9') ||
      c == '.')
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_triple_op (int c0, int c1, int c2) const
{
  return (c0 == 'a' && c1 == 'n' && c2 == 'd' && _boundary23) ||
         (c0 == 'x' && c1 == 'o' && c2 == 'r' && _boundary23) ||
         (c0 == '!' && c1 == '=' && c2 == '=');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_double_op (int c0, int c1, int c2) const
{
  return (c0 == '=' && c1 == '=')                ||
         (c0 == '!' && c1 == '=')                ||
         (c0 == '<' && c1 == '=')                ||
         (c0 == '>' && c1 == '=')                ||
         (c0 == 'o' && c1 == 'r' && _boundary12) ||
         (c0 == '|' && c1 == '|')                ||
         (c0 == '&' && c1 == '&')                ||
         (c0 == '!' && c1 == '~');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_hex_digit (int c) const
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::decode_escape (int c) const
{
  switch (c)
  {
  case 'b':  return 0x08;
  case 'f':  return 0x0C;
  case 'n':  return 0x0A;
  case 'r':  return 0x0D;
  case 't':  return 0x09;
  case 'v':  return 0x0B;
  case '\'': return 0x27;
  case '"':  return 0x22;
  default:   return c;
  }
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::hex_to_int (int c) const
{
       if (c >= '0' && c <= '9') return (c - '0');
  else if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
  else                           return (c - 'A' + 10);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::hex_to_int (int c0, int c1) const
{
  return (hex_to_int (c0) << 4) + hex_to_int (c1);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::hex_to_int (int c0, int c1, int c2, int c3) const
{
  return (hex_to_int (c0) << 12) +
         (hex_to_int (c1) << 8)  +
         (hex_to_int (c2) << 4)  +
          hex_to_int (c3);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::shift ()
{
  _n0 = _n1;
  _n1 = _n2;
  _n2 = _n3;
  _n3 = utf8_next_char (_input, _i);
  ++_shift_counter;

  // Detect type boundaries between characters.
  _boundary01 = boundary (_n0, _n1);
  _boundary12 = boundary (_n1, _n2);
  _boundary23 = boundary (_n2, _n3);
}

////////////////////////////////////////////////////////////////////////////////
