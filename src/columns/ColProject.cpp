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
#include <ColProject.h>
#include <Context.h>
#include <Eval.h>
#include <Variant.h>
#include <Lexer.h>
#include <Filter.h>
#include <shared.h>
#include <format.h>
#include <utf8.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
ColumnProject::ColumnProject ()
{
  _name      = "project";
  _style     = "full";
  _label     = "Project";
  _styles    = {"full", "parent", "indented"};
  _examples  = {"home.garden",
                "home",
                "  home.garden"};
  _hyphenate = Context::getContext ().config.getBoolean ("hyphenate");
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnProject::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;
  if (task.has (_name))
  {
    std::string project = task.get (_name);

    if (_style == "parent")
    {
      auto period = project.find ('.');
      if (period != std::string::npos)
        project = project.substr (0, period);
    }
    else if (_style == "indented")
    {
      project = indentProject (project, "  ", '.');
    }

    minimum = longestWord (project);
    maximum = utf8_width (project);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnProject::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    std::string project = task.get (_name);
    if (_style == "parent")
    {
      auto period = project.find ('.');
      if (period != std::string::npos)
        project = project.substr (0, period);
    }
    else if (_style == "indented")
    {
      project = indentProject (project, "  ", '.');
    }

    std::vector <std::string> raw;
    wrapText (raw, project, width, _hyphenate);

    for (const auto& i : raw)
      renderStringLeft (lines, width, color, i);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnProject::modify (Task& task, const std::string& value)
{
  std::string label = "  [1;37;43mMODIFICATION[0m ";

  // Only if it's a DOM ref, eval it first.
  Lexer lexer (value);
  std::string domRef;
  Lexer::Type type;
  if (lexer.token (domRef, type) &&
      type == Lexer::Type::dom)
  {
    try
    {
      Eval e;
      e.addSource (domSource);

      Variant v;
      e.evaluateInfixExpression (value, v);
      task.set (_name, (std::string) v);
      Context::getContext ().debug (label + _name + " <-- '" + (std::string) v + "' <-- '" + value + '\'');
    }
    catch (const std::string& e)
    {
      // If the expression failed because it didn't look like an expression,
      // simply store it as-is.
      if (e == "The value is not an expression.")
      {
        task.set (_name, value);
        Context::getContext ().debug (label + _name + " <-- '" + value + '\'');
      }
      else
        throw;
    }
  }
  else
  {
    task.set (_name, value);
    Context::getContext ().debug (label + _name + " <-- '" + value + '\'');
  }
}

////////////////////////////////////////////////////////////////////////////////
