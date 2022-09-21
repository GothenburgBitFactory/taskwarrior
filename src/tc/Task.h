////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2022, Dustin J. Mitchell, Tomas Babej, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_TC_TASK
#define INCLUDED_TC_TASK

#include <string>
#include <functional>
#include <memory>
#include <map>
#include "tc/ffi.h"

namespace tc {
  class Replica;

  enum Status {
    Pending = tc::ffi::TC_STATUS_PENDING,
    Completed = tc::ffi::TC_STATUS_COMPLETED,
    Deleted = tc::ffi::TC_STATUS_DELETED,
    Unknown = tc::ffi::TC_STATUS_UNKNOWN,
  };

  // a unique_ptr to a TCReplica which will automatically free the value when
  // it goes out of scope.
  using unique_tctask_ptr = std::unique_ptr<
    tc::ffi::TCTask,
    std::function<void(tc::ffi::TCTask*)>>;


  // Task wraps the TCTask type, managing its memory, errors, and so on.
  //
  // Except as noted, method names match the suffix to `tc_task_..`.
  class Task
  {
  protected:
    // Tasks may only be created by tc::Replica
    friend class tc::Replica;
    explicit Task (tc::ffi::TCTask *);

  public:
    // This object "owns" inner, so copy is not allowed.
    Task (const Task &) = delete;
    Task &operator=(const Task &) = delete;

    // Explicit move constructor and assignment
    Task (Task &&) noexcept;
    Task &operator=(Task &&) noexcept;

// TODO: void tc_task_to_mut(struct TCTask *task, struct TCReplica *tcreplica);
// TODO: void tc_task_to_immut(struct TCTask *task);
    std::string get_uuid () const;
    Status get_status () const;
    std::map <std::string, std::string> get_taskmap() const;
    std::string get_description() const;
// TODO: time_t tc_task_get_entry(struct TCTask *task);
// TODO: time_t tc_task_get_wait(struct TCTask *task);
// TODO: time_t tc_task_get_modified(struct TCTask *task);
// TODO: bool tc_task_is_waiting(struct TCTask *task);
// TODO: bool tc_task_is_active(struct TCTask *task);
// TODO: bool tc_task_has_tag(struct TCTask *task, struct TCString tag);
// TODO: struct TCStringList tc_task_get_tags(struct TCTask *task);
// TODO: struct TCAnnotationList tc_task_get_annotations(struct TCTask *task);
// TODO: struct TCString tc_task_get_uda(struct TCTask *task, struct TCString ns, struct TCString key);
// TODO: struct TCString tc_task_get_legacy_uda(struct TCTask *task, struct TCString key);
// TODO: struct TCUdaList tc_task_get_udas(struct TCTask *task);
// TODO: struct TCUdaList tc_task_get_legacy_udas(struct TCTask *task);
// TODO: TCResult tc_task_set_status(struct TCTask *task, enum TCStatus status);
// TODO: TCResult tc_task_set_description(struct TCTask *task, struct TCString description);
// TODO: TCResult tc_task_set_entry(struct TCTask *task, time_t entry);
// TODO: TCResult tc_task_set_wait(struct TCTask *task, time_t wait);
// TODO: TCResult tc_task_set_modified(struct TCTask *task, time_t modified);
// TODO: TCResult tc_task_start(struct TCTask *task);
// TODO: TCResult tc_task_stop(struct TCTask *task);
// TODO: TCResult tc_task_done(struct TCTask *task);
// TODO: TCResult tc_task_delete(struct TCTask *task);
// TODO: TCResult tc_task_add_tag(struct TCTask *task, struct TCString tag);
// TODO: TCResult tc_task_remove_tag(struct TCTask *task, struct TCString tag);
// TODO: TCResult tc_task_add_annotation(struct TCTask *task, struct TCAnnotation *annotation);
// TODO: TCResult tc_task_remove_annotation(struct TCTask *task, int64_t entry);
// TODO: TCResult tc_task_set_uda(struct TCTask *task,
// TODO: TCResult tc_task_remove_uda(struct TCTask *task, struct TCString ns, struct TCString key);
// TODO: TCResult tc_task_set_legacy_uda(struct TCTask *task, struct TCString key, struct TCString value);
// TODO: TCResult tc_task_remove_legacy_uda(struct TCTask *task, struct TCString key);

  private:
    unique_tctask_ptr inner;

    std::string task_error () const; // tc_task_error
  };
}

// TODO: struct TCTask *tc_task_list_take(struct TCTaskList *tasks, size_t index);
// TODO: void tc_task_list_free(struct TCTaskList *tasks);

#endif
////////////////////////////////////////////////////////////////////////////////
