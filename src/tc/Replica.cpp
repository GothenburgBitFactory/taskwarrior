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
#include "tc/Replica.h"
#include "tc/Task.h"
#include "tc/Server.h"
#include "tc/WorkingSet.h"
#include "tc/util.h"
#include <iostream>

using namespace tc::ffi;

////////////////////////////////////////////////////////////////////////////////
tc::ReplicaGuard::ReplicaGuard (Replica &replica, Task &task) :
  replica(replica),
  task(task)
{
  // "steal" the reference from the Replica and store it locally, so that any
  // attempt to use the Replica will fail
  tcreplica = replica.inner.release();
  task.to_mut(tcreplica);
}

////////////////////////////////////////////////////////////////////////////////
tc::ReplicaGuard::~ReplicaGuard ()
{
  task.to_immut();
  // return the reference to the Replica.
  replica.inner.reset(tcreplica);
}

////////////////////////////////////////////////////////////////////////////////
tc::Replica::Replica ()
{
  inner = unique_tcreplica_ptr (
      tc_replica_new_in_memory (),
      [](TCReplica* rep) { tc_replica_free (rep); });
}

////////////////////////////////////////////////////////////////////////////////
tc::Replica::Replica (Replica &&other) noexcept
{
  // move inner from other
  inner = unique_tcreplica_ptr (
      other.inner.release (),
      [](TCReplica* rep) { tc_replica_free (rep); });
}

////////////////////////////////////////////////////////////////////////////////
tc::Replica& tc::Replica::operator= (Replica &&other) noexcept
{
  if (this != &other) {
    // move inner from other
    inner = unique_tcreplica_ptr (
        other.inner.release (),
        [](TCReplica* rep) { tc_replica_free (rep); });
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
tc::Replica::Replica (const std::string& dir, bool create_if_missing)
{
  TCString path = tc_string_borrow (dir.c_str ());
  TCString error;
  auto tcreplica = tc_replica_new_on_disk (path, create_if_missing, &error);
  if (!tcreplica) {
    auto errmsg = format ("Could not create replica at {1}: {2}", dir, tc_string_content (&error));
    tc_string_free (&error);
    throw errmsg;
  }
  inner = unique_tcreplica_ptr (
      tcreplica,
      [](TCReplica* rep) { tc_replica_free (rep); });
}

////////////////////////////////////////////////////////////////////////////////
tc::WorkingSet tc::Replica::working_set ()
{
  TCWorkingSet *tcws = tc_replica_working_set (&*inner);
  if (!tcws) {
    throw replica_error ();
  }
  return WorkingSet {tcws};
}

////////////////////////////////////////////////////////////////////////////////
std::optional<tc::Task> tc::Replica::get_task (const std::string &uuid)
{
  TCTask *tctask = tc_replica_get_task (&*inner, uuid2tc (uuid));
  if (!tctask) {
    auto error = tc_replica_error (&*inner);
    if (error.ptr) {
      throw replica_error (error);
    } else {
      return std::nullopt;
    }
  }
  return std::make_optional (Task (tctask));
}

////////////////////////////////////////////////////////////////////////////////
tc::Task tc::Replica::new_task (tc::Status status, const std::string &description)
{
  TCTask *tctask = tc_replica_new_task (&*inner, (tc::ffi::TCStatus)status, string2tc (description));
  if (!tctask) {
    throw replica_error ();
  }
  return Task (tctask);
}

////////////////////////////////////////////////////////////////////////////////
tc::Task tc::Replica::import_task_with_uuid (const std::string &uuid)
{
  TCTask *tctask = tc_replica_import_task_with_uuid (&*inner, uuid2tc (uuid));
  if (!tctask) {
    throw replica_error ();
  }
  return Task (tctask);
}

////////////////////////////////////////////////////////////////////////////////
void tc::Replica::sync (Server server, bool avoid_snapshots)
{
  // The server remains owned by this function, per tc_replica_sync docs.
  auto res = tc_replica_sync (&*inner, server.inner.get(), avoid_snapshots);
  if (res != TC_RESULT_OK) {
    throw replica_error ();
  }
}

////////////////////////////////////////////////////////////////////////////////
TCReplicaOpList tc::Replica::get_undo_ops ()
{
  return tc_replica_get_undo_ops(&*inner);
}

////////////////////////////////////////////////////////////////////////////////
void tc::Replica::commit_undo_ops (TCReplicaOpList tc_undo_ops, int32_t *undone_out)
{
  auto res = tc_replica_commit_undo_ops (&*inner, tc_undo_ops, undone_out);
  if (res != TC_RESULT_OK) {
    throw replica_error ();
  }
}

////////////////////////////////////////////////////////////////////////////////
void tc::Replica::free_replica_ops (TCReplicaOpList tc_undo_ops)
{
  tc_replica_op_list_free(&tc_undo_ops);
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Replica::get_op_uuid(TCReplicaOp &tc_replica_op) const
{
  TCString uuid = tc_replica_op_get_uuid(&tc_replica_op);
  return tc2string(uuid);
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Replica::get_op_property(TCReplicaOp &tc_replica_op) const
{
  TCString property = tc_replica_op_get_property(&tc_replica_op);
  return tc2string(property);
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Replica::get_op_value(TCReplicaOp &tc_replica_op) const
{
  TCString value = tc_replica_op_get_value(&tc_replica_op);
  return tc2string(value);
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Replica::get_op_old_value(TCReplicaOp &tc_replica_op) const
{
  TCString old_value = tc_replica_op_get_old_value(&tc_replica_op);
  return tc2string(old_value);
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Replica::get_op_timestamp(TCReplicaOp &tc_replica_op) const
{
  TCString timestamp = tc_replica_op_get_timestamp(&tc_replica_op);
  return tc2string(timestamp);
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Replica::get_op_old_task_description(TCReplicaOp &tc_replica_op) const
{
  TCString description = tc_replica_op_get_old_task_description(&tc_replica_op);
  return tc2string(description);
}

////////////////////////////////////////////////////////////////////////////////
int64_t tc::Replica::num_local_operations ()
{
  auto num = tc_replica_num_local_operations (&*inner);
  if (num < 0) {
    throw replica_error ();
  }
  return num;
}

////////////////////////////////////////////////////////////////////////////////
int64_t tc::Replica::num_undo_points ()
{
  auto num = tc_replica_num_undo_points (&*inner);
  if (num < 0) {
    throw replica_error ();
  }
  return num;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<tc::Task> tc::Replica::all_tasks ()
{
  TCTaskList tasks = tc_replica_all_tasks (&*inner);
  if (!tasks.items) {
    throw replica_error ();
  }

  std::vector <Task> all;
  all.reserve (tasks.len);
  for (size_t i = 0; i < tasks.len; i++) {
    auto tctask = tc_task_list_take (&tasks, i);
    if (tctask) {
      all.push_back (Task (tctask));
    }
  }

  return all;
}

////////////////////////////////////////////////////////////////////////////////
void tc::Replica::rebuild_working_set (bool force)
{
  auto res = tc_replica_rebuild_working_set (&*inner, force);
  if (res != TC_RESULT_OK) {
    throw replica_error ();
  }
}

////////////////////////////////////////////////////////////////////////////////
tc::ReplicaGuard tc::Replica::mutate_task (tc::Task &task) {
  return ReplicaGuard(*this, task);
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Replica::replica_error () {
  return replica_error (tc_replica_error (&*inner));
}

////////////////////////////////////////////////////////////////////////////////
std::string tc::Replica::replica_error (TCString error) {
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
