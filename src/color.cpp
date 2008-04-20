////////////////////////////////////////////////////////////////////////////////
// Copyright 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include "color.h"

////////////////////////////////////////////////////////////////////////////////
std::string Text::colorName (Text::color c)
{
  switch (c)
  {
  case black:    return "black";
  case red:      return "red";
  case green:    return "green";
  case yellow:   return "yellow";
  case blue:     return "blue";
  case magenta:  return "magenta";
  case cyan:     return "cyan";
  case white:    return "white";
  case nocolor:  return "";
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
Text::color Text::colorCode (const std::string& c)
{
  if (c == "black")   return black;
  if (c == "red")     return red;
  if (c == "green")   return green;
  if (c == "yellow")  return yellow;
  if (c == "blue")    return blue;
  if (c == "magenta") return magenta;
  if (c == "cyan")    return cyan;
  if (c == "white")   return white;

  return nocolor;
}

////////////////////////////////////////////////////////////////////////////////
std::string Text::attrName (Text::attr a)
{
  switch (a)
  {
  case underline: return "underline";
  case normal:    return "";
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
Text::attr Text::attrCode (const std::string& a)
{
  if (a == "underline") return underline;

  return normal;
}

////////////////////////////////////////////////////////////////////////////////

