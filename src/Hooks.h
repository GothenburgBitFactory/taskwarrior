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

#ifndef INCLUDED_HOOKS
#define INCLUDED_HOOKS

#include <vector>
#include <string>
#include <Task.h>

class Hooks
{
public:
  Hooks () = default;
  void initialize ();
  bool enable (bool);
  void onLaunch () const;
  void onExit () const;
  void onAdd (Task&) const;
  void onModify (const Task&, Task&) const;
  std::vector <std::string> list () const;

private:
  std::vector <std::string> scripts (const std::string&) const;
  void separateOutput (const std::vector <std::string>&, std::vector <std::string>&, std::vector <std::string>&) const;
  bool isJSON (const std::string&) const;
  void assertValidJSON (const std::vector <std::string>&) const;
  void assertNTasks (const std::vector <std::string>&, unsigned int) const;
  void assertSameTask (const std::vector <std::string>&, const Task&) const;
  void assertFeedback (const std::vector <std::string>&) const;
  std::vector <std::string>& buildHookScriptArgs (std::vector <std::string>&) const;
  int callHookScript (const std::string&, const std::vector <std::string>&, std::vector <std::string>&) const;

private:
  bool                      _enabled {true};
  int                       _debug   {0};
  std::vector <std::string> _scripts {};
};

#endif
