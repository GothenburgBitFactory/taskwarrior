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
#include "tc/Server.h"
#include "tc/util.h"

using namespace tc::ffi;

////////////////////////////////////////////////////////////////////////////////
tc::Server::Server (const std::string &server_dir)
{
  TCString tc_server_dir = tc_string_borrow (server_dir.c_str ());
  TCString error;
  auto tcserver = tc_server_new_local (tc_server_dir, &error);
  if (!tcserver) {
    auto errmsg = format ("Could not configure local server at {1}: {2}",
        server_dir, tc_string_content (&error));
    tc_string_free (&error);
    throw errmsg;
  }
  inner = unique_tcserver_ptr (
      tcserver,
      [](TCServer* rep) { tc_server_free (rep); });
}

////////////////////////////////////////////////////////////////////////////////
tc::Server::Server (const std::string &origin, const std::string &client_key, const std::string &encryption_secret)
{
  TCString tc_origin = tc_string_borrow (origin.c_str ());

  TCString tc_client_key_str = tc_string_borrow (client_key.c_str ());

  TCString tc_encryption_secret = tc_string_borrow (encryption_secret.c_str ());

  TCUuid tc_client_key;
  if (tc_uuid_from_str(tc_client_key_str, &tc_client_key) != TC_RESULT_OK) {
    tc_string_free(&tc_origin);
    tc_string_free(&tc_encryption_secret);
    throw "client_key must be a valid UUID";
  }

  TCString error;
  auto tcserver = tc_server_new_remote (tc_origin, tc_client_key, tc_encryption_secret, &error);
  if (!tcserver) {
    auto errmsg = format ("Could not configure connection to server at {1}: {2}",
        origin, tc_string_content (&error));
    tc_string_free (&error);
    throw errmsg;
  }
  inner = unique_tcserver_ptr (
      tcserver,
      [](TCServer* rep) { tc_server_free (rep); });
}

////////////////////////////////////////////////////////////////////////////////
tc::Server::Server (tc::Server &&other) noexcept
{
  // move inner from other
  inner = unique_tcserver_ptr (
      other.inner.release (),
      [](TCServer* rep) { tc_server_free (rep); });
}

////////////////////////////////////////////////////////////////////////////////
tc::Server& tc::Server::operator= (tc::Server &&other) noexcept
{
  if (this != &other) {
    // move inner from other
    inner = unique_tcserver_ptr (
        other.inner.release (),
        [](TCServer* rep) { tc_server_free (rep); });
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
