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

#include <cmake.h>
#include <format.h>
#include "tc/WorkingSet.h"
#include "tc/Task.h"
#include "tc/util.h"

using namespace tc::ffi;

////////////////////////////////////////////////////////////////////////////////
tc::WorkingSet::WorkingSet (WorkingSet &&other) noexcept
{
  // move inner from other
  inner = unique_tcws_ptr (
      other.inner.release (),
      [](TCWorkingSet* ws) { tc_working_set_free (ws); });
}

////////////////////////////////////////////////////////////////////////////////
tc::WorkingSet& tc::WorkingSet::operator= (WorkingSet &&other) noexcept
{
  if (this != &other) {
    // move inner from other
    inner = unique_tcws_ptr (
        other.inner.release (),
        [](TCWorkingSet* ws) { tc_working_set_free (ws); });
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
tc::WorkingSet::WorkingSet (tc::ffi::TCWorkingSet* tcws)
{
  inner = unique_tcws_ptr (
      tcws,
      [](TCWorkingSet* ws) { tc_working_set_free (ws); });
}

////////////////////////////////////////////////////////////////////////////////
size_t tc::WorkingSet::len () const noexcept
{
  return tc_working_set_len (&*inner);
}

////////////////////////////////////////////////////////////////////////////////
size_t tc::WorkingSet::largest_index () const noexcept
{
  return tc_working_set_largest_index (&*inner);
}

////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> tc::WorkingSet::by_index (size_t index) const noexcept
{
  TCUuid uuid;
  if (tc_working_set_by_index (&*inner, index, &uuid)) {
    return std::make_optional (tc2uuid (uuid));
  } else {
    return std::nullopt;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> tc::WorkingSet::by_uuid (const std::string &uuid) const noexcept
{
  auto index = tc_working_set_by_uuid (&*inner, uuid2tc (uuid));
  if (index > 0) {
    return std::make_optional (index);
  } else {
    return std::nullopt;
  }
}

////////////////////////////////////////////////////////////////////////////////
