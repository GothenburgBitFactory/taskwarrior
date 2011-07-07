////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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

#include <sstream>
#include <ViewText.h>
#include <Context.h>
#include <Color.h>
#include <text.h>
#include <i18n.h>
#include <CmdColor.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdColor::CmdColor ()
{
  _keyword     = "colors";
  _usage       = "task colors [sample | legend]";
  _description = "Displays all possible colors, a named sample, or a legend "
                 "containing all currently defined colors.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdColor::execute (std::string& output)
{
  int rc = 0;

  // Get the non-attribute, non-fancy command line arguments.
  bool legend = false;
  Arguments words = context.args.extract_simple_words ();
  std::vector <Triple>::iterator word;
  for (word = words.begin (); word != words.end (); ++word)
    if (closeEnough ("legend", word->_first))
      legend = true;

  std::stringstream out;
  if (context.color ())
  {
    // If the description contains 'legend', show all the colors currently in
    // use.
    if (legend)
    {
      out << "\nHere are the colors currently in use:\n";

      std::vector <std::string> all;
      context.config.all (all);

      ViewText view;
      view.width (context.getWidth ());
      view.add (Column::factory ("string", "Color"));
      view.add (Column::factory ("string", "Definition"));

      std::vector <std::string>::iterator item;
      for (item = all.begin (); item != all.end (); ++item)
      {
        // Skip items with 'color' in their name, that are not referring to
        // actual colors.
        if (*item != "_forcecolor" &&
            *item != "color"       &&
            item->find ("color") == 0)
        {
          Color color (context.config.get (*item));
          int row = view.addRow ();
          view.set (row, 0, *item, color);
          view.set (row, 1, context.config.get (*item), color);
        }
      }

      out << view.render ()
          << "\n";
    }

    // If there is something in the description, then assume that is a color,
    // and display it as a sample.
    else if (words.size ())
    {
      Color one    ("black on bright yellow");
      Color two    ("underline cyan on bright blue");
      Color three  ("color214 on color202");
      Color four   ("rgb150 on rgb020");
      Color five   ("underline grey10 on grey3");
      Color six    ("red on color173");

      std::string swatch;
      for (word = words.begin (); word != words.end (); ++word)
      {
        if (word != words.begin ())
          swatch += " ";

        swatch += word->_first;
      }

      Color sample (swatch);

      out << "\n"
          << "Use this command to see how colors are displayed by your terminal.\n\n"
          << "\n"
          << "16-color usage (supports underline, bold text, bright background):\n"
          << "  " << one.colorize ("task color black on bright yellow")            << "\n"
          << "  " << two.colorize ("task color underline cyan on bright blue")     << "\n"
          << "\n"
          << "256-color usage (supports underline):\n"
          << "  " << three.colorize ("task color color214 on color202")            << "\n"
          << "  " << four.colorize ("task color rgb150 on rgb020")                 << "\n"
          << "  " << five.colorize ("task color underline grey10 on grey3")        << "\n"
          << "  " << six.colorize ("task color red on color173")                   << "\n"
          << "\n"
          << "Your sample:"                                                        << "\n"
          << "  " << sample.colorize ("task color " + swatch)                      << "\n\n";
    }

    // Show all supported colors.  Possibly show some unsupported ones too.
    else
    {
      out << "\n"
          << "Basic colors"
          << "\n"
          << " " << Color::colorize (" black ",   "black")
          << " " << Color::colorize (" red ",     "red")
          << " " << Color::colorize (" blue ",    "blue")
          << " " << Color::colorize (" green ",   "green")
          << " " << Color::colorize (" magenta ", "magenta")
          << " " << Color::colorize (" cyan ",    "cyan")
          << " " << Color::colorize (" yellow ",  "yellow")
          << " " << Color::colorize (" white ",   "white")
          << "\n"
          << " " << Color::colorize (" black ",   "white on black")
          << " " << Color::colorize (" red ",     "white on red")
          << " " << Color::colorize (" blue ",    "white on blue")
          << " " << Color::colorize (" green ",   "black on green")
          << " " << Color::colorize (" magenta ", "black on magenta")
          << " " << Color::colorize (" cyan ",    "black on cyan")
          << " " << Color::colorize (" yellow ",  "black on yellow")
          << " " << Color::colorize (" white ",   "black on white")
          << "\n\n";

      out << "Effects"
          << "\n"
          << " " << Color::colorize (" red ",               "red")
          << " " << Color::colorize (" bold red ",          "bold red")
          << " " << Color::colorize (" underline on blue ", "underline on blue")
          << " " << Color::colorize (" on green ",          "black on green")
          << " " << Color::colorize (" on bright green ",   "black on bright green")
          << " " << Color::colorize (" inverse ",           "inverse")
          << "\n\n";

      // 16 system colors.
      out << "color0 - color15"
          << "\n"
          << "  0 1 2 . . .\n";
      for (int r = 0; r < 2; ++r)
      {
        out << "  ";
        for (int c = 0; c < 8; ++c)
        {
          std::stringstream s;
          s << "on color" << (r*8 + c);
          out << Color::colorize ("  ", s.str ());
        }

        out << "\n";
      }

      out << "          . . . 15\n\n";

      // Color cube.
      out << "Color cube rgb"
          << Color::colorize ("0", "bold red")
          << Color::colorize ("0", "bold green")
          << Color::colorize ("0", "bold blue")
          << " - rgb"
          << Color::colorize ("5", "bold red")
          << Color::colorize ("5", "bold green")
          << Color::colorize ("5", "bold blue")
          << " (also color16 - color231)"
          << "\n"
          << "  " << Color::colorize ("0            "
                                      "1            "
                                      "2            "
                                      "3            "
                                      "4            "
                                      "5", "bold red")
          << "\n"
          << "  " << Color::colorize ("0 1 2 3 4 5  "
                                      "0 1 2 3 4 5  "
                                      "0 1 2 3 4 5  "
                                      "0 1 2 3 4 5  "
                                      "0 1 2 3 4 5  "
                                      "0 1 2 3 4 5", "bold blue")
          << "\n";

      char label [12];
      for (int g = 0; g < 6; ++g)
      {
        sprintf (label, " %d", g);
        out << Color::colorize (label, "bold green");
        for (int r = 0; r < 6; ++r)
        {
          for (int b = 0; b < 6; ++b)
          {
            std::stringstream s;
            s << "on rgb" << r << g << b;
            out << Color::colorize ("  ", s.str ());
          }

          out << " ";
        }

        out << "\n";
      }

      out << "\n";

      // Grey ramp.
      out << "Gray ramp gray0 - gray23 (also color232 - color255)\n"
          << "  0 1 2 . . .                             . . . 23\n"
          << "  ";
      for (int g = 0; g < 24; ++g)
      {
        std::stringstream s;
        s << "on gray" << g;
        out << Color::colorize ("  ", s.str ());
      }

      out << "\n\nTry running 'task color white on red'.\n\n";
    }
  }
  else
  {
    out << "Color is currently turned off in your .taskrc file.  To enable "
           "color, remove the line 'color=off', or change the 'off' to 'on'.\n";
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
