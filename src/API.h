////////////////////////////////////////////////////////////////////////////////
// Taskwarrior Lua API
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_API
#define INCLUDED_API
#define L10N                                           // Localization complete.

#include <cmake.h>
#ifdef HAVE_LIBLUA

#include <vector>
#include <string>
#include <Task.h>

extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

class API
{
public:
  API ();
  API (const API&);
  API& operator= (const API&);
  ~API ();

  void initialize ();
  bool callProgramHook (const std::string&, const std::string&);
  bool callTaskHook    (const std::string&, const std::string&, Task&);

private:
  void loadFile (const std::string&);

public:
  lua_State* _state;
  std::vector <std::string> _loaded;

  // Context for the API.
//  std::vector <Task> all;
  Task _current;
//  std::string& name;
//  std::string& value;
};

#endif
#endif
////////////////////////////////////////////////////////////////////////////////
