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
#include <ColTags.h>
#include <algorithm>
#include <Context.h>
#include <Eval.h>
#include <Variant.h>
#include <Filter.h>
#include <Dates.h>
#include <text.h>
#include <i18n.h>
#include <utf8.h>
#include <main.h>

extern Context context;
extern Task& contextTask;

////////////////////////////////////////////////////////////////////////////////
ColumnTags::ColumnTags ()
{
  _name      = "tags";
  _style     = "list";
  _label     = STRING_COLUMN_LABEL_TAGS;
  _styles    = {"list", "indicator", "count"};
  _examples  = {STRING_COLUMN_EXAMPLES_TAGS,
                context.config.get ("tag.indicator"),
                "[2]"};
  _hyphenate = false;
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnTags::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "indicator" &&
      _label == STRING_COLUMN_LABEL_TAGS)
    _label = _label.substr (0, context.config.get ("tag.indicator").length ());

  else if (_style == "count" &&
            _label == STRING_COLUMN_LABEL_TAGS)
    _label = STRING_COLUMN_LABEL_TAG;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnTags::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;

  if (task.has (_name))
  {
    if (_style == "indicator")
    {
      if (task.has ("tags"))
        minimum = maximum = utf8_width (context.config.get ("tag.indicator"));
      else
        minimum = maximum = 0;
    }
    else if (_style == "count")
    {
      minimum = maximum = 3;
    }
    else if (_style == "default" ||
             _style == "list")
    {
      std::string tags = task.get (_name);

      // Find the widest tag.
      if (tags.find (',') != std::string::npos)
      {
        std::vector <std::string> all;
        split (all, tags, ',');
        for (const auto& tag : all)
        {
          auto length = utf8_width (tag);
          if (length > minimum)
            minimum = length;
        }

        maximum = utf8_width (tags);
      }

      // No need to split a single tag.
      else
        minimum = maximum = utf8_width (tags);
    }
    else
      throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTags::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    std::string tags = task.get (_name);
    if (_style == "default" ||
        _style == "list")
    {
      if (tags.find (',') != std::string::npos)
      {
        std::vector <std::string> all;
        split (all, tags, ',');
        std::sort (all.begin (), all.end ());
        join (tags, " ", all);

        all.clear ();
        wrapText (all, tags, width, _hyphenate);

        for (const auto& i : all)
          renderStringLeft (lines, width, color, i);
      }
      else
        renderStringLeft (lines, width, color, tags);
    }
    else if (_style == "indicator")
    {
      renderStringRight (lines, width, color, context.config.get ("tag.indicator"));
    }
    else if (_style == "count")
    {
      std::vector <std::string> all;
      split (all, tags, ',');
      renderStringRight (lines, width, color, "[" + format (static_cast <int> (all.size ())) + "]");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTags::modify (Task& task, const std::string& value)
{
  std::string label = "  [1;37;43mMODIFICATION[0m ";

  // TW-1701
  task.set ("tags", "");

  std::vector <std::string> tags;
  split (tags, value, ',');

  for (auto& tag : tags)
  {
    // If it's a DOM ref, eval it first.
    Lexer lexer (tag);
    std::string domRef;
    Lexer::Type type;
    if (lexer.token (domRef, type) &&
        type == Lexer::Type::dom)
    {
      Eval e;
      e.addSource (domSource);
      e.addSource (namedDates);
      contextTask = task;

      Variant v;
      e.evaluateInfixExpression (value, v);
      task.addTag ((std::string) v);
      context.debug (label + "tags <-- '" + (std::string) v + "' <-- '" + tag + "'");
    }
    else
    {
      task.addTag (tag);
      context.debug (label + "tags <-- '" + tag + "'");
    }

    feedback_special_tags (task, tag);
  }
}

////////////////////////////////////////////////////////////////////////////////
