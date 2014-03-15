////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_E9
#define INCLUDED_E9

#include <string>
#include <A3.h>
#include <Task.h>
#include <RX.h>

class E9
{
public:
  E9 (const A3&);
  ~E9 ();

  bool evalFilter (const Task&);
  std::string evalExpression (const Task&);

private:
  void eval (const Task&, std::vector <Arg>&);
  bool eval_match (Arg&, Arg&, bool);

  // Unary.
  void operator_not      (Arg&, Arg&);
  void operator_negate   (Arg&, Arg&);

  // Binary.
  void operator_and      (Arg&, Arg&, Arg&);
  void operator_or       (Arg&, Arg&, Arg&);
  void operator_xor      (Arg&, Arg&, Arg&);
  void operator_lt       (Arg&, Arg&, Arg&);
  void operator_lte      (Arg&, Arg&, Arg&);
  void operator_gte      (Arg&, Arg&, Arg&);
  void operator_gt       (Arg&, Arg&, Arg&);
  void operator_inequal  (Arg&, Arg&, Arg&, bool);
  void operator_equal    (Arg&, Arg&, Arg&, bool);
  void operator_match    (Arg&, Arg&, Arg&, bool, const Task&);
  void operator_nomatch  (Arg&, Arg&, Arg&, bool, const Task&);
  void operator_hastag   (Arg&, Arg&, bool, const Task&);

  const Arg coerce (const Arg&, const Arg::type);
  bool get_bool (const Arg&);

private:
  std::vector <Arg> _terms;
  std::map <std::string, RX> _regexes;
  std::string _dateformat;
  bool _dom;
};

#endif
////////////////////////////////////////////////////////////////////////////////
