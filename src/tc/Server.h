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

#ifndef INCLUDED_TC_SERVER
#define INCLUDED_TC_SERVER

#include <string>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include "tc/ffi.h"

namespace tc {
  // a unique_ptr to a TCServer which will automatically free the value when
  // it goes out of scope.
  using unique_tcserver_ptr = std::unique_ptr<
    tc::ffi::TCServer,
    std::function<void(tc::ffi::TCServer*)>>;

  // Server wraps the TCServer type, managing its memory, errors, and so on.
  //
  // Except as noted, method names match the suffix to `tc_server_..`.
  class Server
  {
  public:
    // Construct a null server
    Server () = default;

    // Construct a local server (tc_server_new_local).
    static Server new_local (const std::string& server_dir);

    // Construct a remote server (tc_server_new_sync).
    static Server new_sync (const std::string &origin, const std::string &client_id, const std::string &encryption_secret);

    // Construct a GCP server (tc_server_new_gcp).
    static Server new_gcp (const std::string &bucket, const std::string &encryption_secret);

    // This object "owns" inner, so copy is not allowed.
    Server (const Server &) = delete;
    Server &operator=(const Server &) = delete;

    // Explicit move constructor and assignment
    Server (Server &&) noexcept;
    Server &operator=(Server &&) noexcept;

  protected:
    Server (unique_tcserver_ptr inner) : inner(std::move(inner)) {};

    unique_tcserver_ptr inner;

    // Replica accesses the inner pointer to call tc_replica_sync
    friend class Replica;

    // construct an error message from the given string.
    std::string server_error (tc::ffi::TCString string);
  };
}


#endif
////////////////////////////////////////////////////////////////////////////////
