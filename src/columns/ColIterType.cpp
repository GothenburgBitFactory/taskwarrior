////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2026, Adam Milner.
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
// cmake needs to come first

#include <ColIterType.h>
#include <utf8.h>

////////////////////////////////////////////////////////////////////////////////
ColumnIterType::ColumnIterType() {
  _name = "iter_type";
  _style = "default";
  _label = "Iter type";
  _modifiable = true;
  _styles = {"default"};
  _examples = {"fixed", "fixed+", "chained"};
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnIterType::measure(Task& task, unsigned int& minimum, unsigned int& maximum) {
  minimum = maximum = 0;
  if (task.has(_name)) minimum = maximum = utf8_width(task.get(_name));
}

////////////////////////////////////////////////////////////////////////////////
void ColumnIterType::render(std::vector<std::string>& lines, Task& task, int width, Color& color) {
  if (task.has(_name)) renderStringLeft(lines, width, color, task.get(_name));
}

////////////////////////////////////////////////////////////////////////////////
// Accepts the iteration types and short forms understood by TaskChampion.
bool ColumnIterType::validate(const std::string& input) const {
  return input == "fixed" || input == "fx" || input == "fixed+" || input == "f+" || input == "fp" ||
         input == "chained" || input == "ch";
}

////////////////////////////////////////////////////////////////////////////////
