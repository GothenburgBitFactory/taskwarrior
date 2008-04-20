////////////////////////////////////////////////////////////////////////////////
// Copyright 2008, Paul Beckingham.
// All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_COLOR
#define INCLUDED_COLOR

namespace Text
{
  enum color  {nocolor = 0, black, red, green, yellow, blue, magenta, cyan, white};
  enum attr   {normal = 0, underline};

  std::string colorName (Text::color);
  Text::color colorCode (const std::string&);

  std::string attrName (Text::attr);
  Text::attr attrCode (const std::string&);
}

#endif

////////////////////////////////////////////////////////////////////////////////
