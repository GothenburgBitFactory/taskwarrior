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

#ifndef INCLUDED_TC_REPLICA
#define INCLUDED_TC_REPLICA

#include <string>
#include <functional>
#include <memory>
#include <optional>
#include "tc/ffi.h"
#include "tc/Task.h"

namespace tc {
  class Task;
  class WorkingSet;

  // a unique_ptr to a TCReplica which will automatically free the value when
  // it goes out of scope.
  using unique_tcreplica_ptr = std::unique_ptr<
    tc::ffi::TCReplica,
    std::function<void(tc::ffi::TCReplica*)>>;

  // Replica wraps the TCReplica type, managing its memory, errors, and so on.
  //
  // Except as noted, method names match the suffix to `tc_replica_..`.
  class Replica
  {
  public:
    Replica (); // tc_replica_new_in_memory
    Replica (const std::string& dir); // tc_replica_new_on_disk

    // This object "owns" inner, so copy is not allowed.
    Replica (const Replica &) = delete;
    Replica &operator=(const Replica &) = delete;

    // Explicit move constructor and assignment
    Replica (Replica &&) noexcept;
    Replica &operator=(Replica &&) noexcept;

    std::vector<tc::Task> all_tasks ();
// TODO: struct TCUuidList tc_replica_all_task_uuids(struct TCReplica *rep);
    tc::WorkingSet working_set ();
    std::optional<tc::Task> get_task (const std::string &uuid);
    tc::Task new_task (Status status, const std::string &description);
// TODO: struct TCTask *tc_replica_import_task_with_uuid(struct TCReplica *rep, struct TCUuid tcuuid);
// TODO: TCResult tc_replica_sync(struct TCReplica *rep, struct TCServer *server, bool avoid_snapshots);
// TODO: TCResult tc_replica_undo(struct TCReplica *rep, int32_t *undone_out);
// TODO: TCResult tc_replica_add_undo_point(struct TCReplica *rep, bool force);
    void rebuild_working_set ();
  private:
    unique_tcreplica_ptr inner;

    // construct an error message from tc_replica_error, or from the given
    // string retrieved from tc_replica_error.
    std::string replica_error ();
    std::string replica_error (tc::ffi::TCString string);
  };
}

#endif
////////////////////////////////////////////////////////////////////////////////
