////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2022, Dustin J. Mitchell
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

#ifndef INCLUDED_TC_WORKINGSET
#define INCLUDED_TC_WORKINGSET

#include <string>
#include <functional>
#include <memory>
#include <optional>
#include "tc/ffi.h"
#include "tc/Task.h"

namespace tc {
  class Task;

  // a unique_ptr to a TCWorkingSet which will automatically free the value when
  // it goes out of scope.
  using unique_tcws_ptr = std::unique_ptr<
    tc::ffi::TCWorkingSet,
    std::function<void(tc::ffi::TCWorkingSet*)>>;

  // WorkingSet wraps the TCWorkingSet type, managing its memory, errors, and so on.
  //
  // Except as noted, method names match the suffix to `tc_working_set_..`.
  class WorkingSet
  {
  protected:
    friend class tc::Replica;
    WorkingSet (tc::ffi::TCWorkingSet*); // via tc_replica_working_set

  public:
    // This object "owns" inner, so copy is not allowed.
    WorkingSet (const WorkingSet &) = delete;
    WorkingSet &operator=(const WorkingSet &) = delete;

    // Explicit move constructor and assignment
    WorkingSet (WorkingSet &&) noexcept;
    WorkingSet &operator=(WorkingSet &&) noexcept;

    size_t len () const noexcept; // tc_working_set_len
    size_t largest_index () const noexcept; // tc_working_set_largest_index
    std::optional<std::string> by_index (size_t index) const noexcept; // tc_working_set_by_index
    std::optional<size_t> by_uuid (const std::string &index) const noexcept; // tc_working_set_by_uuid

  private:
    unique_tcws_ptr inner;
  };
}

#endif
////////////////////////////////////////////////////////////////////////////////
