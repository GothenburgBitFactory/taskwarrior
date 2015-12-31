////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Göteborg Bit Factory.
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
#include <test.h>
#include <Eval.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
// A few hard-coded symbols.
bool get (const std::string& name, Variant& value)
{
  if (name == "x")
    value = Variant (true);
  else
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (52);

  // Test the source independently.
  Variant v;
  t.notok (get ("-", v),                   "true <-- get(-)");

  t.ok (get ("x", v),                      "true <-- get(x)");
  t.is (v.type (), Variant::type_boolean,  "get(x) --> boolean");
  t.is (v.get_bool (), true,               "get(x) --> true");

  Eval e;
  e.addSource (get);
  Variant result;
  e.evaluatePostfixExpression ("x", result);
  t.is (result.type (), Variant::type_boolean, "postfix 'x' --> boolean");
  t.is (result.get_bool (), true,              "postfix 'x' --> true");

  e.evaluatePostfixExpression ("pi", result);
  t.is (result.type (), Variant::type_real,    "postfix 'pi' --> real");
  t.is (result.get_real (), 3.141592, 0.00001, "postfix 'pi' --> 3.14159265");

  e.evaluatePostfixExpression ("foo", result);
  t.is (result.type (), Variant::type_string,  "postfix 'foo' --> string");
  t.is (result.get_string (), "foo",           "postfix 'foo' --> 'foo'");

  // Simple infix arithmetic.
  e.evaluateInfixExpression ("1+2", result);
  t.is (result.type (), Variant::type_integer, "infix '1 + 2' --> integer");
  t.is (result.get_integer (), 3,              "infix '1 + 2' --> 3");

  // Simple postfix arithmetic.
  e.evaluatePostfixExpression ("1 2 +", result);
  t.is (result.type (), Variant::type_integer, "postfix '1 2 +' --> integer");
  t.is (result.get_integer (), 3,              "postfix '1 2 +' --> 3");

  e.evaluatePostfixExpression ("1 2 -", result);
  t.is (result.type (), Variant::type_integer, "postfix '1 2 -' --> integer");
  t.is (result.get_integer (), -1,             "postfix '1 2 -' --> -1");

  e.evaluatePostfixExpression ("2 3 *", result);
  t.is (result.type (), Variant::type_integer, "postfix '2 3 *' --> integer");
  t.is (result.get_integer (), 6,              "postfix '2 3 *' --> 6");

  e.evaluatePostfixExpression ("5 2 /", result);
  t.is (result.type (), Variant::type_integer, "postfix '5 2 /' --> integer");
  t.is (result.get_integer (), 2,              "postfix '5 2 /' --> 2");

  e.evaluatePostfixExpression ("5 2 /", result);
  t.is (result.type (), Variant::type_integer, "postfix '5 2 *' --> integer");
  t.is (result.get_integer (), 2,              "postfix '5 2 *' --> 2");

  // Simple postfix unary operator.
  e.evaluatePostfixExpression ("0 !", result);
  t.is (result.type (), Variant::type_boolean, "postfix '0 !' --> boolean");
  t.is (result.get_bool (), true,              "postfix '0 !' --> true");

  e.evaluatePostfixExpression ("1 !", result);
  t.is (result.type (), Variant::type_boolean, "postfix '1 !' --> boolean");
  t.is (result.get_bool (), false,             "postfix '1 !' --> false");

  // Type promotion simple postfix arithmetic.
  e.evaluatePostfixExpression ("1 2.3 +", result);
  t.is (result.type (), Variant::type_real,    "postfix '1 2.3 +' --> real");
  t.is (result.get_real (), 3.3,               "postfix '1 2.3 +' --> 3.3");

  e.evaluatePostfixExpression ("5 2.0 /", result);
  t.is (result.type (), Variant::type_real,    "postfix '5 2.0 /' --> integer");
  t.is (result.get_real (), 2.5,               "postfix '5 2.0 /' --> 2.5");

  // Simple logic.
  e.evaluatePostfixExpression ("0 0 ||", result);
  t.is (result.type (), Variant::type_boolean, "postfix '0 0 ||' --> boolean");
  t.is (result.get_bool (), false,             "postfix '0 0 ||' --> false");

  e.evaluatePostfixExpression ("0 1 ||", result);
  t.is (result.type (), Variant::type_boolean, "postfix '0 1 ||' --> boolean");
  t.is (result.get_bool (), true,              "postfix '0 1 ||' --> true");

  e.evaluatePostfixExpression ("1 0 ||", result);
  t.is (result.type (), Variant::type_boolean, "postfix '1 0 ||' --> boolean");
  t.is (result.get_bool (), true,              "postfix '1 0 ||' --> true");

  e.evaluatePostfixExpression ("1 1 ||", result);
  t.is (result.type (), Variant::type_boolean, "postfix '1 1 ||' --> boolean");
  t.is (result.get_bool (), true,              "postfix '1 1 ||' --> true");

  e.evaluateInfixExpression ("2*3+1", result);
  t.is (result.type (), Variant::type_integer, "infix '2*3+1' --> integer");
  t.is (result.get_integer (), 7,              "infix '2*3+1' --> 7");

  // TW-1254 - Unary minus support.
  e.evaluateInfixExpression ("2- -3", result);
  t.is (result.type (), Variant::type_integer, "infix '2- -3' --> integer");
  t.is (result.get_integer (), 5,              "infix '2- -3' --> 5");

  //e.debug ();
  e.evaluateInfixExpression ("!false", result);
  t.is (result.type (), Variant::type_boolean, "infix '!false' --> boolean");
  t.is (result.get_bool (), true,              "infix '!false' --> true");

  e.evaluateInfixExpression ("!true", result);
  t.is (result.type (), Variant::type_boolean, "infix '!true' --> boolean");
  t.is (result.get_bool (), false,             "infix '!true' --> false");

  // _neg_
  e.evaluateInfixExpression ("- 1", result);
  t.is (result.type (), Variant::type_integer, "infix '- 1' --> integer");
  t.is (result.get_integer (), -1,             "infix '- 1' --> -1");

  e.evaluateInfixExpression ("- 1.2", result);
  t.is (result.type (), Variant::type_real,    "infix '- 1.2' --> real");
  t.is (result.get_real (), -1.2,              "infix '- 1.2' --> -1.2");

  e.evaluateInfixExpression ("- 2days", result);
  t.is (result.type (), Variant::type_duration, "infix '- 2days' --> duration");
  t.is (result.get_duration (), -86400*2,      "infix '- 2days' --> -86400 * 2");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
