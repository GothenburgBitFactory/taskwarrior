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

#include <CmdSync.h>
#include <Color.h>
#include <Context.h>
#include <Filter.h>
#include <format.h>
#include <inttypes.h>
#include <shared.h>
#include <signal.h>
#include <taskchampion-cpp/lib.h>
#include <util.h>

#include <sstream>

////////////////////////////////////////////////////////////////////////////////
CmdSync::CmdSync() {
  _keyword = "synchronize";
  _usage = "task          synchronize [initialize]";
  _description = "Synchronizes data with the Taskserver";
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
int CmdSync::execute(std::string& output) {
  int status = 0;

  Context& context = Context::getContext();
  auto& replica = context.tdb2.replica();
  std::stringstream out;
  bool avoid_snapshots = false;
  bool verbose = Context::getContext().verbose("sync");

  // If no server is set up, quit.
  std::string origin = Context::getContext().config.get("sync.server.origin");
  std::string url = Context::getContext().config.get("sync.server.url");
  std::string server_dir = Context::getContext().config.get("sync.local.server_dir");
  std::string client_id = Context::getContext().config.get("sync.server.client_id");
  std::string gcp_credential_path = Context::getContext().config.get("sync.gcp.credential_path");
  std::string gcp_bucket = Context::getContext().config.get("sync.gcp.bucket");
  std::string encryption_secret = Context::getContext().config.get("sync.encryption_secret");

  // sync.server.origin is a deprecated synonym for sync.server.url
  std::string server_url = url == "" ? origin : url;
  if (origin != "") {
    out << "sync.server.origin is deprecated. Use sync.server.url instead.\n";
  }

  if (server_dir != "") {
    if (verbose) {
      out << format("Syncing with {1}", server_dir) << '\n';
    }
    replica->sync_to_local(server_dir, avoid_snapshots);
  } else if (gcp_bucket != "") {
    if (encryption_secret == "") {
      throw std::string("sync.encryption_secret is required");
    }
    if (verbose) {
      out << format("Syncing with GCP bucket {1}", gcp_bucket) << '\n';
    }
    replica->sync_to_gcp(gcp_bucket, gcp_credential_path, encryption_secret, avoid_snapshots);
  } else if (server_url != "") {
    if (client_id == "" || encryption_secret == "") {
      throw std::string("sync.server.client_id and sync.encryption_secret are required");
    }
    if (verbose) {
      out << format("Syncing with sync server at {1}", server_url) << '\n';
    }
    replica->sync_to_remote(server_url, tc::uuid_from_string(client_id), encryption_secret,
                            avoid_snapshots);
  } else {
    throw std::string("No sync.* settings are configured. See task-sync(5).");
  }

  if (context.config.getBoolean("purge.on-sync")) {
    context.tdb2.expire_tasks();
  }

  output = out.str();
  return status;
}

////////////////////////////////////////////////////////////////////////////////
