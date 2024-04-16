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
#include <CmdSync.h>
#include <sstream>
#include <inttypes.h>
#include <signal.h>
#include <Context.h>
#include <Filter.h>
#include <Color.h>
#include <shared.h>
#include <format.h>
#include <util.h>
#include "tc/Server.h"

////////////////////////////////////////////////////////////////////////////////
CmdSync::CmdSync ()
{
  _keyword               = "synchronize";
  _usage                 = "task          synchronize [initialize]";
  _description           = "Synchronizes data with the Taskserver";
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::migration;
}

////////////////////////////////////////////////////////////////////////////////
int CmdSync::execute (std::string& output)
{
  int status = 0;

  tc::Server server;
  std::string server_ident;

  // If no server is set up, quit.
  std::string origin = Context::getContext ().config.get ("sync.server.origin");
  std::string server_dir = Context::getContext ().config.get ("sync.local.server_dir");
  std::string gcp_credential_path = Context::getContext ().config.get ("sync.gcp.credential_path");
  std::string gcp_bucket = Context::getContext ().config.get ("sync.gcp.bucket");
  std::string encryption_secret = Context::getContext ().config.get ("sync.encryption_secret");
  if (server_dir != "") {
    server = tc::Server::new_local (server_dir);
    server_ident = server_dir;
  } else if (gcp_bucket != "") {
    if (encryption_secret == "") {
      throw std::string ("sync.encryption_secret is required");
    }
    server = tc::Server::new_gcp (gcp_bucket, gcp_credential_path, encryption_secret);
    std::ostringstream os;
    os << "GCP bucket " << gcp_bucket;
    server_ident = os.str();
  } else if (origin != "") {
    std::string client_id = Context::getContext ().config.get ("sync.server.client_id");
    if (client_id == "" || encryption_secret == "") {
      throw std::string ("sync.server.client_id and sync.encryption_secret are required");
    }
    server = tc::Server::new_sync (origin, client_id, encryption_secret);
    std::ostringstream os;
    os << "Sync server at " << origin;
    server_ident = os.str();
  } else {
    throw std::string ("No sync.* settings are configured. See task-sync(5).");
  }

  std::stringstream out;
  if (Context::getContext ().verbose ("sync"))
    out << format ("Syncing with {1}", server_ident)
        << '\n';

  Context::getContext ().tdb2.sync(std::move(server), false);

  output = out.str ();
  return status;
}

////////////////////////////////////////////////////////////////////////////////
