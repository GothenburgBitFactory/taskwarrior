////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_DOM
#define INCLUDED_DOM

#include <string>
#include <Variant.h>
#include <Task.h>

// 2017-04-22 Deprecated, use DOM::get.
bool getDOM (const std::string&, Variant&);
bool getDOM (const std::string&, const Task&, Variant&);

class DOM
{
public:
  void addSource (const std::string&, bool (*)(const std::string&, Variant&));
  bool valid (const std::string&) const;
  Variant get (const Task&, const std::string&) const;
  Variant get (const std::string&) const;
  int count () const;
  std::shared_ptr <DOM> find (const std::string&);
  std::string dump () const;

private:
  std::vector <std::string> decomposeReference (const std::string&) const;
  std::string dumpNode (const std::shared_ptr <DOM>, int) const;

private:
  std::string                                              _name     {"Unknown"};
  std::shared_ptr <bool (*)(const std::string&, Variant&)> _provider {nullptr};
  std::vector <std::shared_ptr <DOM>>                      _branches {};
};

#endif
