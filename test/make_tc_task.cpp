////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2025, Dustin J. Mitchell
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

#include <CmdInfo.h>
#include <main.h>
#include <stdlib.h>
#include <taskchampion-cpp/lib.h>
#include <test.h>
#include <util.h>

#include <iostream>
#include <limits>

#include "format.h"

namespace {

////////////////////////////////////////////////////////////////////////////////
int usage() {
  std::cerr << "USAGE: make_tc_task DATADIR KEY=VALUE ..\n";
  return 1;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
  if (!--argc) {
    return usage();
  }
  char *datadir = *++argv;

  auto replica = tc::new_replica_on_disk(datadir, /*create_if_missing=*/true, /*read_write=*/true);
  auto uuid = tc::uuid_v4();
  auto operations = tc::new_operations();
  auto task = tc::create_task(uuid, operations);

  while (--argc) {
    std::string arg = *++argv;
    size_t eq_idx = arg.find('=');
    if (eq_idx == std::string::npos) {
      return usage();
    }
    std::string property = arg.substr(0, eq_idx);
    std::string value = arg.substr(eq_idx + 1);
    task->update(property, value, operations);
  }
  replica->commit_operations(std::move(operations));

  std::cout << static_cast<std::string>(uuid.to_string()) << "\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
