////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
// cmake.h include header must come first

#include <CmdColor.h>
#include <Color.h>
#include <Context.h>
#include <Table.h>
#include <format.h>
#include <shared.h>

#include <sstream>

////////////////////////////////////////////////////////////////////////////////
CmdColor::CmdColor() {
  _keyword = "colors";
  _usage = "task          colors [sample | legend]";
  _description = "All colors, a sample, or a legend";
  _read_only = true;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category = Command::Category::misc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdColor::execute(std::string& output) {
  int rc = 0;

  // Get the non-attribute, non-fancy command line arguments.
  auto legend = false;
  auto words = Context::getContext().cli2.getWords();
  for (auto& word : words)
    if (closeEnough("legend", word)) legend = true;

  std::stringstream out;
  if (Context::getContext().color()) {
    // If the description contains 'legend', show all the colors currently in
    // use.
    if (legend) {
      out << "\nHere are the colors currently in use:\n";

      Table view;
      view.width(Context::getContext().getWidth());
      if (Context::getContext().config.getBoolean("color")) view.forceColor();
      view.add("Color");
      view.add("Definition");

      for (auto& item : Context::getContext().config) {
        // Skip items with 'color' in their name, that are not referring to
        // actual colors.
        if (item.first != "_forcecolor" && item.first != "color" && item.first.find("color") == 0) {
          Color color(Context::getContext().config.get(item.first));
          int row = view.addRow();
          view.set(row, 0, item.first, color);
          view.set(row, 1, item.second, color);
        }
      }

      out << view.render() << '\n';
    }

    // If there is something in the description, then assume that is a color,
    // and display it as a sample.
    else if (words.size()) {
      Color one("black on bright yellow");
      Color two("underline cyan on bright blue");
      Color three("color214 on color202");
      Color four("rgb150 on rgb020");
      Color five("underline grey10 on grey3");
      Color six("red on color173");

      std::string swatch;
      for (auto word = words.begin(); word != words.end(); ++word) {
        if (word != words.begin()) swatch += ' ';

        swatch += *word;
      }

      Color sample(swatch);

      out << '\n'
          << "Use this command to see how colors are displayed by your terminal.\n"
          << "\n\n"
          << "16-color usage (supports underline, bold text, bright background):\n"
          << "  " << one.colorize("task color black on bright yellow") << '\n'
          << "  " << two.colorize("task color underline cyan on bright blue") << '\n'
          << '\n'
          << "256-color usage (supports underline):\n"
          << "  " << three.colorize("task color color214 on color202") << '\n'
          << "  " << four.colorize("task color rgb150 on rgb020") << '\n'
          << "  " << five.colorize("task color underline grey10 on grey3") << '\n'
          << "  " << six.colorize("task color red on color173") << '\n'
          << '\n'
          << "Your sample:\n\n"
          << "  " << sample.colorize("task color " + swatch) << "\n\n";
    }

    // Show all supported colors.  Possibly show some unsupported ones too.
    else {
      out << '\n'
          << "Basic colors\n"
          << ' ' << Color::colorize(" black ", "black") << ' ' << Color::colorize(" red ", "red")
          << ' ' << Color::colorize(" blue ", "blue") << ' ' << Color::colorize(" green ", "green")
          << ' ' << Color::colorize(" magenta ", "magenta") << ' '
          << Color::colorize(" cyan ", "cyan") << ' ' << Color::colorize(" yellow ", "yellow")
          << ' ' << Color::colorize(" white ", "white") << '\n'
          << ' ' << Color::colorize(" black ", "white on black") << ' '
          << Color::colorize(" red ", "white on red") << ' '
          << Color::colorize(" blue ", "white on blue") << ' '
          << Color::colorize(" green ", "black on green") << ' '
          << Color::colorize(" magenta ", "black on magenta") << ' '
          << Color::colorize(" cyan ", "black on cyan") << ' '
          << Color::colorize(" yellow ", "black on yellow") << ' '
          << Color::colorize(" white ", "black on white") << "\n\n";

      out << "Effects\n"
          << ' ' << Color::colorize(" red ", "red") << ' '
          << Color::colorize(" bold red ", "bold red") << ' '
          << Color::colorize(" underline on blue ", "underline on blue") << ' '
          << Color::colorize(" on green ", "black on green") << ' '
          << Color::colorize(" on bright green ", "black on bright green") << ' '
          << Color::colorize(" inverse ", "inverse") << "\n\n";

      // 16 system colors.
      out << "color0 - color15" << '\n' << "  0 1 2 . . .\n";
      for (int r = 0; r < 2; ++r) {
        out << "  ";
        for (int c = 0; c < 8; ++c) {
          std::stringstream s;
          s << "on color" << (r * 8 + c);
          out << Color::colorize("  ", s.str());
        }

        out << '\n';
      }

      out << "          . . . 15\n\n";

      // Color cube.
      out << "Color cube rgb" << Color::colorize("0", "bold red")
          << Color::colorize("0", "bold green") << Color::colorize("0", "bold blue") << " - rgb"
          << Color::colorize("5", "bold red") << Color::colorize("5", "bold green")
          << Color::colorize("5", "bold blue") << " (also color16 - color231)" << '\n'
          << "  "
          << Color::colorize(
                 "0            "
                 "1            "
                 "2            "
                 "3            "
                 "4            "
                 "5",
                 "bold red")
          << '\n'
          << "  "
          << Color::colorize(
                 "0 1 2 3 4 5  "
                 "0 1 2 3 4 5  "
                 "0 1 2 3 4 5  "
                 "0 1 2 3 4 5  "
                 "0 1 2 3 4 5  "
                 "0 1 2 3 4 5",
                 "bold blue")
          << '\n';

      char label[12];
      for (int g = 0; g < 6; ++g) {
        snprintf(label, 12, " %d", g);
        out << Color::colorize(label, "bold green");
        for (int r = 0; r < 6; ++r) {
          for (int b = 0; b < 6; ++b) {
            std::stringstream s;
            s << "on rgb" << r << g << b;
            out << Color::colorize("  ", s.str());
          }

          out << ' ';
        }

        out << '\n';
      }

      out << '\n';

      // Grey ramp.
      out << "Gray ramp gray0 - gray23 (also color232 - color255)\n"
          << "  0 1 2 . . .                             . . . 23\n"
          << "  ";
      for (int g = 0; g < 24; ++g) {
        std::stringstream s;
        s << "on gray" << g;
        out << Color::colorize("  ", s.str());
      }

      out << "\n\n"
          << "Try running 'task color white on red'.\n"
          << '\n';
    }
  } else {
    out << "Color is currently turned off in your .taskrc file.  To enable color, remove the line "
           "'color=off', or change the 'off' to 'on'.\n";
    rc = 1;
  }

  output = out.str();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
