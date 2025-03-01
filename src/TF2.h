////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2024, Tomas Babej, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_TF2
#define INCLUDED_TF2

#include <FS.h>
#include <Task.h>

#include <map>
#include <string>
#include <vector>

// TF2 Class represents a single 2.x-style file in the task database.
//
// This is only used for importing tasks from 2.x. It only reads format 4, based
// on a stripped-down version of the TF2 class from v2.6.2.
class TF2 {
 public:
  TF2();
  ~TF2();

  void target(const std::string&);

  const std::vector<std::map<std::string, std::string>>& get_tasks();

  std::map<std::string, std::string> load_task(const std::string&);
  void load_tasks();
  void load_lines();
  const std::string decode(const std::string& value) const;

  bool _loaded_tasks;
  bool _loaded_lines;
  std::vector<std::map<std::string, std::string>> _tasks;
  std::vector<std::string> _lines;
  File _file;
};

#endif
////////////////////////////////////////////////////////////////////////////////
