////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

// Hook class representing a single hook, which is just a three-way map.
class Hook
{
public:
  Hook ();
  Hook (const std::string&, const std::string&, const std::string&);
  Hook (const Hook&);
  Hook& operator= (const Hook&);

public:
  std::string _event;
  std::string _file;
  std::string _function;
};

// Hooks class for managing the loading and calling of hook functions.
class Hooks
{
public:
  Hooks ();                         // Default constructor
  ~Hooks ();                        // Destructor
  Hooks (const Hooks&);             // Deliberately unimplemented
  Hooks& operator= (const Hooks&);  // Deliberately unimplemented

  void initialize ();

  bool trigger (const std::string&);                                   // Program
  bool trigger (const std::string&, Task&);                            // Task

private:
  bool validProgramEvent (const std::string&);
  bool validTaskEvent (const std::string&);

private:
  std::vector <Hook> _all;           // All current hooks.

  std::vector <std::string> _validProgramEvents;
  std::vector <std::string> _validTaskEvents;
};

#endif
////////////////////////////////////////////////////////////////////////////////
