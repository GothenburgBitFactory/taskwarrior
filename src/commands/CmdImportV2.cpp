////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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

#include <CmdImportV2.h>
#include <CmdModify.h>
#include <Context.h>
#include <TF2.h>
#include <format.h>
#include <shared.h>
#include <util.h>

#include <iostream>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
CmdImportV2::CmdImportV2() {
  _keyword = "import-v2";
  _usage = "task          import-v2";
  _description = "Imports Taskwarrior v2.x files";
  _read_only = false;
  _displays_id = false;
  _needs_gc = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category = Command::Category::migration;
}

////////////////////////////////////////////////////////////////////////////////
int CmdImportV2::execute(std::string&) {
  std::vector<std::map<std::string, std::string>> task_data;

  std::string location = (Context::getContext().data_dir);
  File pending_file = File(location + "/pending.data");
  if (pending_file.exists()) {
    TF2 pending_tf;
    pending_tf.target(pending_file);
    auto& pending_tasks = pending_tf.get_tasks();
    task_data.insert(task_data.end(), pending_tasks.begin(), pending_tasks.end());
  }
  File completed_file = File(location + "/completed.data");
  if (completed_file.exists()) {
    TF2 completed_tf;
    completed_tf.target(completed_file);
    auto& completed_tasks = completed_tf.get_tasks();
    task_data.insert(task_data.end(), completed_tasks.begin(), completed_tasks.end());
  }

  auto count = import(task_data);

  Context::getContext().footnote(
      format("Imported {1} tasks from `*.data` files. You may now delete these files.", count));
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int CmdImportV2::import(const std::vector<std::map<std::string, std::string>>& task_data) {
  auto count = 0;
  const std::string uuid_key = "uuid";
  const std::string id_key = "id";
  const std::string descr_key = "description";
  auto& replica = Context::getContext().tdb2.replica();
  rust::Vec<tc::Operation> ops;
  tc::add_undo_point(ops);

  for (auto& task : task_data) {
    auto uuid_iter = task.find(uuid_key);
    if (uuid_iter == task.end()) {
      std::cout << " err  - Task with no UUID\n";
      continue;
    }
    auto uuid_str = uuid_iter->second;
    auto uuid = tc::uuid_from_string(uuid_str);

    bool added_task = false;
    auto maybe_task_data = replica->get_task_data(uuid);
    auto task_data = maybe_task_data.is_some() ? maybe_task_data.take() : [&]() {
      added_task = true;
      return tc::create_task(uuid, ops);
    }();

    for (auto& attr : task) {
      if (attr.first == uuid_key || attr.first == id_key) {
        continue;
      }
      task_data->update(attr.first, attr.second, ops);
    }
    count++;

    if (added_task) {
      std::cout << " add ";
    } else {
      std::cout << " mod ";
    }
    std::cout << uuid_str << ' ';
    if (auto descr_iter = task.find(descr_key); descr_iter != task.end()) {
      std::cout << descr_iter->second;
    } else {
      std::cout << "(no description)";
    }
    std::cout << "\n";
  }

  replica->commit_operations(std::move(ops));
  return count;
}

////////////////////////////////////////////////////////////////////////////////
