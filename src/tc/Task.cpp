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
#include <assert.h>
#include "tc/Task.h"
#include "tc/util.h"

using namespace tc::ffi;

////////////////////////////////////////////////////////////////////////////////
tc::Task::Task (TCTask *tctask)
{
  inner = unique_tctask_ptr(
      tctask,
      [](TCTask* task) { tc_task_free(task); });
}

////////////////////////////////////////////////////////////////////////////////
tc::Task::Task (Task &&other) noexcept
{
  // move inner from other
  inner = unique_tctask_ptr(
      other.inner.release(),
      [](TCTask* task) { tc_task_free(task); });
}

////////////////////////////////////////////////////////////////////////////////
tc::Task& tc::Task::operator= (Task &&other) noexcept
{
  if (this != &other) {
    // move inner from other
    inner = unique_tctask_ptr(
        other.inner.release(),
        [](TCTask* task) { tc_task_free(task); });
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Task::get_uuid () const
{
  auto uuid = tc_task_get_uuid(&*inner);
  return tc2uuid(uuid);
}

////////////////////////////////////////////////////////////////////////////////
tc::Status tc::Task::get_status () const
{
  auto status = tc_task_get_status(&*inner);
  return tc::Status(status);
}

////////////////////////////////////////////////////////////////////////////////
std::map <std::string, std::string> tc::Task::get_taskmap () const
{
  TCKVList kv = tc_task_get_taskmap (&*inner);
  if (!kv.items) {
    throw task_error ();
  }

  std::map<std::string, std::string> taskmap;
  for (size_t i = 0; i < kv.len; i++) {
    auto k = tc2string_clone(kv.items[i].key);
    auto v = tc2string_clone(kv.items[i].value);
    taskmap[k] = v;
  }

  return taskmap;
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Task::get_description () const
{
  auto desc = tc_task_get_description(&*inner);
  return tc2string(desc);
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Task::task_error () const {
  TCString error = tc_task_error (&*inner);
  std::string errmsg;
  if (!error.ptr) {
    errmsg = std::string ("Unknown TaskChampion error");
  } else {
    errmsg = std::string (tc_string_content (&error));
  }
  tc_string_free (&error);
  return errmsg;
}

////////////////////////////////////////////////////////////////////////////////
