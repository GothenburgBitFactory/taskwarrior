////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2025, Tomas Babej, Paul Beckingham, Federico Hernandez,
//                        Tobias Predel.
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

#ifndef INCLUDED_OPERATION
#define INCLUDED_OPERATION

#include <taskchampion-cpp/lib.h>

#include <optional>
#include <vector>

// Representation of a TaskChampion operation.
//
// This class wraps `tc::Operation&` and thus cannot outlive that underlying
// type.
class Operation {
 public:
  explicit Operation(const tc::Operation &);

  Operation(const Operation &other) = default;
  Operation &operator=(const Operation &other);

  // Create a vector of Operations given the result of `Replica::get_undo_operations` or
  // `Replica::get_task_operations`. The resulting vector must not outlive the input `rust::Vec`.
  static std::vector<Operation> operations(const rust::Vec<tc::Operation> &);

  // Methods from the underlying `tc::Operation`.
  bool is_create() const { return op->is_create(); }
  bool is_update() const { return op->is_update(); }
  bool is_delete() const { return op->is_delete(); }
  bool is_undo_point() const { return op->is_undo_point(); }
  std::string get_uuid() const { return std::string(op->get_uuid().to_string()); }
  ::rust::Vec<::tc::PropValuePair> get_old_task() const { return op->get_old_task(); };
  std::string get_property() const {
    std::string value;
    op->get_property(value);
    return value;
  }
  std::optional<std::string> get_value() const {
    std::optional<std::string> value{std::string()};
    if (!op->get_value(value.value())) {
      value = std::nullopt;
    }
    return value;
  }
  std::optional<std::string> get_old_value() const {
    std::optional<std::string> old_value{std::string()};
    if (!op->get_old_value(old_value.value())) {
      old_value = std::nullopt;
    }
    return old_value;
  }
  time_t get_timestamp() const { return static_cast<time_t>(op->get_timestamp()); }

  // Define a partial order on Operations:
  //  - Create < Update < Delete < UndoPoint
  //  - Given two updates, sort by timestamp
  bool operator<(const Operation &other) const;

 private:
  const tc::Operation *op;
};

#endif
////////////////////////////////////////////////////////////////////////////////
