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
#include <vector>
#include "tc/ffi.h"
#include "tc/Task.h"

namespace tc {
  class Task;
  class WorkingSet;
  class Server;

  // a unique_ptr to a TCReplica which will automatically free the value when
  // it goes out of scope.
  using unique_tcreplica_ptr = std::unique_ptr<
    tc::ffi::TCReplica,
    std::function<void(tc::ffi::TCReplica*)>>;

  // ReplicaGuard uses RAII to ensure that a Replica is not accessed while it
  // is mutably borrowed (specifically, to make a task mutable).
  class ReplicaGuard {
  protected:
    friend class Replica;
    explicit ReplicaGuard (Replica &, Task &);

  public:
    ~ReplicaGuard();

    // No moving or copying allowed
    ReplicaGuard (const ReplicaGuard &) = delete;
    ReplicaGuard &operator=(const ReplicaGuard &) = delete;
    ReplicaGuard (ReplicaGuard &&) = delete;
    ReplicaGuard &operator=(Replica &&) = delete;

  private:
    Replica &replica;
    tc::ffi::TCReplica *tcreplica;
    Task &task;
  };

  // Replica wraps the TCReplica type, managing its memory, errors, and so on.
  //
  // Except as noted, method names match the suffix to `tc_replica_..`.
  class Replica
  {
  public:
    Replica (); // tc_replica_new_in_memory
    Replica (const std::string& dir, bool create_if_missing); // tc_replica_new_on_disk

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
    tc::Task import_task_with_uuid (const std::string &uuid);
// TODO: struct TCTask *tc_replica_import_task_with_uuid(struct TCReplica *rep, struct TCUuid tcuuid);
    void sync(Server server, bool avoid_snapshots);
    tc::ffi::TCReplicaOpList get_undo_ops ();
    void commit_undo_ops (tc::ffi::TCReplicaOpList tc_undo_ops, int32_t *undone_out);
    void free_replica_ops (tc::ffi::TCReplicaOpList tc_undo_ops);
    std::string get_op_uuid(tc::ffi::TCReplicaOp &tc_replica_op) const;
    std::string get_op_property(tc::ffi::TCReplicaOp &tc_replica_op) const;
    std::string get_op_value(tc::ffi::TCReplicaOp &tc_replica_op) const;
    std::string get_op_old_value(tc::ffi::TCReplicaOp &tc_replica_op) const;
    std::string get_op_timestamp(tc::ffi::TCReplicaOp &tc_replica_op) const;
    std::string get_op_old_task_description(tc::ffi::TCReplicaOp &tc_replica_op) const;
    int64_t num_local_operations ();
    int64_t num_undo_points ();
// TODO: TCResult tc_replica_add_undo_point(struct TCReplica *rep, bool force);
    void rebuild_working_set (bool force);

    ReplicaGuard mutate_task(tc::Task &);
    void immut_task(tc::Task &);

  protected:
    friend class ReplicaGuard;
    unique_tcreplica_ptr inner;

    // construct an error message from tc_replica_error, or from the given
    // string retrieved from tc_replica_error.
    std::string replica_error ();
    std::string replica_error (tc::ffi::TCString string);
  };
}


#endif
////////////////////////////////////////////////////////////////////////////////
