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
#include <ColTypeString.h>
#include <Context.h>
#include <Eval.h>
#include <Variant.h>
#include <Filter.h>
#include <Dates.h>
#include <text.h>
#include <i18n.h>

extern Context context;
extern Task& contextTask;

////////////////////////////////////////////////////////////////////////////////
ColumnTypeString::ColumnTypeString ()
{
  _type = "string";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTypeString::modify (Task& task, const std::string& value)
{
  std::string label = "  [1;37;43mMODIFICATION[0m ";

  // Only if it's a DOM ref, eval it first.
  Lexer lexer (value);
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
    std::string strValue = (std::string) v;
    if (validate (strValue))
    {
      task.set (_name, strValue);
      context.debug (label + _name + " <-- '" + strValue + "' <-- '" + value + "'");
    }
    else
      throw format (STRING_INVALID_MOD, _name, value);
  }
  else
  {
    if (validate (value))
    {
      task.set (_name, value);
      context.debug (label + _name + " <-- '" + value + "'");
    }
    else
      throw format (STRING_INVALID_MOD, _name, value);
  }
}

////////////////////////////////////////////////////////////////////////////////
