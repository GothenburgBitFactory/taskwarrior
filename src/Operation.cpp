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

#include <cmake.h>
// cmake.h include header must come first

#include <Operation.h>
#include <taskchampion-cpp/lib.h>

#include <vector>

////////////////////////////////////////////////////////////////////////////////
Operation::Operation(const tc::Operation& op) : op(&op) {}

////////////////////////////////////////////////////////////////////////////////
std::vector<Operation> Operation::operations(const rust::Vec<tc::Operation>& operations) {
  return {operations.begin(), operations.end()};
}

////////////////////////////////////////////////////////////////////////////////
Operation& Operation::operator=(const Operation& other) {
  op = other.op;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Operation::operator<(const Operation& other) const {
  if (is_create()) {
    return !other.is_create();
  } else if (is_update()) {
    if (other.is_create()) {
      return false;
    } else if (other.is_update()) {
      return get_timestamp() < other.get_timestamp();
    } else {
      return true;
    }
  } else if (is_delete()) {
    if (other.is_create() || other.is_update() || other.is_delete()) {
      return false;
    } else {
      return true;
    }
  } else if (is_undo_point()) {
    return !other.is_undo_point();
  }
  return false;  // not reachable
}

////////////////////////////////////////////////////////////////////////////////
