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
#include <iostream>
#include <stdlib.h>
#include <Context.h>
#include <Color.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (40 + 256 + 256 + 6*6*6 + 6*6*6 + 1 + 24 + 24 + 3);

  // Names matched to values.
  t.is ((int) Color (""),        (int) Color (Color::nocolor), "''        == Color::nocolor");
  t.is ((int) Color ("black"),   (int) Color (Color::black),   "'black'   == Color::black");
  t.is ((int) Color ("red"),     (int) Color (Color::red),     "'red'     == Color::red");
  t.is ((int) Color ("green"),   (int) Color (Color::green),   "'green'   == Color::green");
  t.is ((int) Color ("yellow"),  (int) Color (Color::yellow),  "'yellow'  == Color::yellow");
  t.is ((int) Color ("blue"),    (int) Color (Color::blue),    "'blue'    == Color::blue");
  t.is ((int) Color ("magenta"), (int) Color (Color::magenta), "'magenta' == Color::magenta");
  t.is ((int) Color ("cyan"),    (int) Color (Color::cyan),    "'cyan'    == Color::cyan");
  t.is ((int) Color ("white"),   (int) Color (Color::white),   "'white'   == Color::white");

  // Auto upgrades.
  Color c ("red on color0");
  t.is ((std::string) c, "color1 on color0", "upgrade red on color0 -> color1 on color0");

  c = Color ("color1 on black");
  t.is ((std::string) c, "color1 on color0", "upgrade color1 on black -> color1 on color0");

  c = Color ("bold red on color0");
  t.is ((std::string) c, "color9 on color0", "upgrade bold red on color0 -> color9 on color0");

  c = Color ("color1 on bright black");
  t.is ((std::string) c, "color1 on color8", "upgrade color1 on bright black -> color1 on color8");

  // Simple blending.
  c = Color ("red");
  c.blend (Color ("on white"));
  t.is ((std::string) c, "red on white", "red + on white -> red on white");

  c = Color ("bold underline red");
  c.blend (Color ("on bright white"));
  t.is ((std::string) c, "bold underline red on bright white", "bold underline red + on bright white -> bold underline red on bright white");

  // Blending with conflicts.
  c = Color ("red on white");
  c.blend (Color ("on blue"));
  t.is ((std::string) c, "red on blue", "red on white + on blue -> red on blue");

  c = Color ("red on white");
  c.blend (Color ("blue on magenta"));
  t.is ((std::string) c, "blue on magenta", "red on white + blue on magenta -> blue on magenta");

  // Blending with upgrades.
  c = Color ("color1 on color0");
  c.blend (Color ("blue"));
  t.is ((std::string) c, "color4 on color0", "color1 on color0 + blue -> color4 on color0");

  // Now the dumb show of every color and its code.
  t.is (Color::colorize ("foo", "red"),                std::string ("\033[31mfoo\033[0m"),       "red                -> ^[[31m");
  t.is (Color::colorize ("foo", "bold red"),           std::string ("\033[1;31mfoo\033[0m"),     "bold red           -> ^[[1;31m");
  t.is (Color::colorize ("foo", "underline red"),      std::string ("\033[4;31mfoo\033[0m"),     "underline red      -> ^[[4;31m");
  t.is (Color::colorize ("foo", "underline bold red"), std::string ("\033[1;4;31mfoo\033[0m"),   "underline bold red -> ^[[1;4;31m");

  // 16-color foregrounds.
  t.is (Color::colorize ("foo", ""),                   std::string ("foo"),                      "''                 -> ''");

  t.is (Color::colorize ("foo", "black"),              std::string ("\033[30mfoo\033[0m"),       "black              -> ^[[30m");
  t.is (Color::colorize ("foo", "red"),                std::string ("\033[31mfoo\033[0m"),       "red                -> ^[[31m");
  t.is (Color::colorize ("foo", "green"),              std::string ("\033[32mfoo\033[0m"),       "green              -> ^[[32m");
  t.is (Color::colorize ("foo", "yellow"),             std::string ("\033[33mfoo\033[0m"),       "yellow             -> ^[[33m");
  t.is (Color::colorize ("foo", "blue"),               std::string ("\033[34mfoo\033[0m"),       "blue               -> ^[[34m");
  t.is (Color::colorize ("foo", "magenta"),            std::string ("\033[35mfoo\033[0m"),       "magenta            -> ^[[35m");
  t.is (Color::colorize ("foo", "cyan"),               std::string ("\033[36mfoo\033[0m"),       "cyan               -> ^[[36m");
  t.is (Color::colorize ("foo", "white"),              std::string ("\033[37mfoo\033[0m"),       "white              -> ^[[37m");

  // 16-color backgrounds.
  t.is (Color::colorize ("foo", "on bright black"),    std::string ("\033[100mfoo\033[0m"),      "on bright black    -> ^[[100m");

  t.is (Color::colorize ("foo", "on black"),           std::string ("\033[40mfoo\033[0m"),       "on black           -> ^[[40m");
  t.is (Color::colorize ("foo", "on red"),             std::string ("\033[41mfoo\033[0m"),       "on red             -> ^[[41m");
  t.is (Color::colorize ("foo", "on green"),           std::string ("\033[42mfoo\033[0m"),       "on green           -> ^[[42m");
  t.is (Color::colorize ("foo", "on yellow"),          std::string ("\033[43mfoo\033[0m"),       "on yellow          -> ^[[43m");
  t.is (Color::colorize ("foo", "on blue"),            std::string ("\033[44mfoo\033[0m"),       "on blue            -> ^[[44m");
  t.is (Color::colorize ("foo", "on magenta"),         std::string ("\033[45mfoo\033[0m"),       "on magenta         -> ^[[45m");
  t.is (Color::colorize ("foo", "on cyan"),            std::string ("\033[46mfoo\033[0m"),       "on cyan            -> ^[[46m");
  t.is (Color::colorize ("foo", "on white"),           std::string ("\033[47mfoo\033[0m"),       "on white           -> ^[[47m");

  // 256-color, basic colors.
  char color [24];
  char codes [64];
  char description [64];
  for (int i = 0; i < 256; ++i)
  {
    sprintf (color,       "color%d", i);
    sprintf (codes,       "\033[38;5;%dmfoo\033[0m", i);
    sprintf (description, "color%d -> ^[[38;5;%dm", i, i);

    t.is (Color::colorize ("foo", color), std::string (codes), description);
  }

  for (int i = 0; i < 256; ++i)
  {
    sprintf (color,       "on color%d", i);
    sprintf (codes,       "\033[48;5;%dmfoo\033[0m", i);
    sprintf (description, "on color%d -> ^[[48;5;%dm", i, i);

    t.is (Color::colorize ("foo", color), std::string (codes), description);
  }

  // RGB Color Cube.
  for (int r = 0; r < 6; ++r)
    for (int g = 0; g < 6; ++g)
      for (int b = 0; b < 6; ++b)
      {
        int code = 16 + (r*36 + g*6 + b);
        sprintf (color,       "rgb%d%d%d", r, g, b);
        sprintf (codes,       "\033[38;5;%dmfoo\033[0m", code);
        sprintf (description, "rgb%d%d%d -> ^[[38;5;%dm", r, g, b, code);

        t.is (Color::colorize ("foo", color), std::string (codes), description);
      }

  for (int r = 0; r < 6; ++r)
    for (int g = 0; g < 6; ++g)
      for (int b = 0; b < 6; ++b)
      {
        int code = 16 + (r*36 + g*6 + b);
        sprintf (color,       "on rgb%d%d%d", r, g, b);
        sprintf (codes,       "\033[48;5;%dmfoo\033[0m", code);
        sprintf (description, "on rgb%d%d%d -> ^[[48;5;%dm", r, g, b, code);

        t.is (Color::colorize ("foo", color), std::string (codes), description);
      }

  // 256-color, grays.
  // grey == gray.
  t.is (Color::colorize ("foo", "grey0"), std::string ("\033[38;5;232mfoo\033[0m"), "grey0 -> ^[[38;5;232m");

  for (int i = 0; i < 24; ++i)
  {
    sprintf (color,       "gray%d", i);
    sprintf (codes,       "\033[38;5;%dmfoo\033[0m", i + 232);
    sprintf (description, "gray%d -> ^[[38;5;%dm", i + 232, i + 232);

    t.is (Color::colorize ("foo", color), std::string (codes), description);
  }

  for (int i = 0; i < 24; ++i)
  {
    sprintf (color,       "on gray%d", i);
    sprintf (codes,       "\033[48;5;%dmfoo\033[0m", i + 232);
    sprintf (description, "on gray%d -> ^[[48;5;%dm", i + 232, i + 232);

    t.is (Color::colorize ("foo", color), std::string (codes), description);
  }

  // std::string Color::strip (const std::string&);
  t.is (Color::strip (""),                  "",    "Color::strip '' -> ''");
  t.is (Color::strip ("foo"),               "foo", "Color::strip 'foo' -> 'foo'");
  t.is (Color::strip ("f\033[1mo\033[0mo"), "foo", "Color::strip 'f<b>o</b>o' -> 'foo'");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
