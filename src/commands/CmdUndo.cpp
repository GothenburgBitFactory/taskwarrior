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

#include <CmdUndo.h>
#include <Context.h>
#include <Operation.h>
#include <Task.h>
#include <shared.h>

#include <iostream>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
CmdUndo::CmdUndo() {
  _keyword = "undo";
  _usage = "task          undo";
  _description = "Reverts the most recent change to a task";
  _read_only = false;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
int CmdUndo::execute(std::string&) {
  auto& replica = Context::getContext().tdb2.replica();
  rust::Vec<tc::Operation> undo_ops = replica->get_undo_operations();
  if (confirm_revert(Operation::operations(undo_ops))) {
    // Note that commit_reversed_operations rebuilds the working set, so that
    // need not be done here.
    if (!replica->commit_reversed_operations(std::move(undo_ops))) {
      std::cout << "Could not undo: other operations have occurred.";
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
bool CmdUndo::confirm_revert(const std::vector<Operation>& undo_ops) {
  // Count non-undo operations
  int ops_count = 0;
  for (auto& op : undo_ops) {
    if (!op.is_undo_point()) {
      ops_count++;
    }
  }
  if (ops_count == 0) {
    std::cout << "No operations to undo.\n";
    return true;
  }

  std::cout << "The following " << ops_count << " operations would be reverted:\n";

  Table view;
  if (Context::getContext().config.getBoolean("obfuscate")) view.obfuscate();
  view.width(Context::getContext().getWidth());
  view.add("Uuid");
  view.add("Modification");

  std::string last_uuid;
  std::stringstream mods;
  for (auto& op : undo_ops) {
    if (op.is_undo_point()) {
      continue;
    }

    if (last_uuid != op.get_uuid()) {
      if (last_uuid.size() != 0) {
        int row = view.addRow();
        view.set(row, 0, last_uuid);
        view.set(row, 1, mods.str());
      }
      last_uuid = op.get_uuid();
      mods = std::stringstream();
    }

    if (op.is_create()) {
      mods << "Create task\n";
    } else if (op.is_delete()) {
      mods << "Delete (purge) task";
    } else if (op.is_update()) {
      auto property = op.get_property();
      auto old_value = op.get_old_value();
      auto value = op.get_value();
      if (Task::isTagAttr(property)) {
        if (value && *value == "x") {
          mods << "Add tag '" << Task::attr2Tag(property) << "'\n";
          continue;
        } else if (!value && old_value && *old_value == "x") {
          mods << "Remove tag '" << Task::attr2Tag(property) << "'\n";
          continue;
        }
      }
      if (old_value && value) {
        mods << "Update property '" << property << "' from '" << *old_value << "' to '" << *value
             << "'\n";
      } else if (old_value) {
        mods << "Delete property '" << property << "' (was '" << *old_value << "')\n";
      } else if (value) {
        mods << "Add property '" << property << "' with value '" << *value << "'\n";
      }
    }
  }
  int row = view.addRow();
  view.set(row, 0, last_uuid);
  view.set(row, 1, mods.str());
  std::cout << view.render() << "\n";

  return !Context::getContext().config.getBoolean("confirmation") ||
         confirm(
             "The undo command is not reversible.  Are you sure you want to revert to the previous "
             "state?");
  return true;
}

////////////////////////////////////////////////////////////////////////////////
