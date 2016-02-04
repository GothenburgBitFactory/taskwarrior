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
#include <ColTypeNumeric.h>
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
ColumnTypeNumeric::ColumnTypeNumeric ()
{
  _type = "numeric";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTypeNumeric::modify (Task& task, const std::string& value)
{
  // Try to evaluate 'value'.  It might work.
  Variant evaluatedValue;
  try
  {
    Eval e;
    e.addSource (domSource);
    e.addSource (namedDates);
    contextTask = task;
    e.evaluateInfixExpression (value, evaluatedValue);
  }

  catch (...)
  {
    evaluatedValue = Variant (value);
  }

  std::string label = "  [1;37;43mMODIFICATION[0m ";
  context.debug (label + _name + " <-- '" + evaluatedValue.get_string () + "' <-- '" + value + "'");

  // If the result is not readily convertible to a numeric value, then this is
  // an error.
  if (evaluatedValue.type () == Variant::type_string)
    throw format (STRING_UDA_NUMERIC, evaluatedValue.get_string ());

  task.set (_name, evaluatedValue);
}

////////////////////////////////////////////////////////////////////////////////
