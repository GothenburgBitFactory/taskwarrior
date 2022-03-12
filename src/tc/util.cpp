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
#include <assert.h>
#include "tc/Replica.h"
#include "tc/Task.h"

using namespace tc::ffi;

namespace tc {
////////////////////////////////////////////////////////////////////////////////
TCString string2tc (const std::string& str)
{
  return tc_string_clone_with_len (str.data (), str.size ());
}

////////////////////////////////////////////////////////////////////////////////
std::string tc2string_clone (const TCString& str)
{
  size_t len;
  auto ptr = tc_string_content_with_len (&str, &len);
  auto rv = std::string (ptr, len);
  return rv;
}

////////////////////////////////////////////////////////////////////////////////
std::string tc2string (TCString& str)
{
  auto rv = tc2string_clone(str);
  tc_string_free (&str);
  return rv;
}

////////////////////////////////////////////////////////////////////////////////
TCUuid uuid2tc(const std::string& str)
{
  TCString tcstr = tc_string_borrow(str.c_str());
  TCUuid rv;
  if (TC_RESULT_OK != tc_uuid_from_str(tcstr, &rv)) {
    throw std::string ("invalid UUID");
  }
  return rv;
}

////////////////////////////////////////////////////////////////////////////////
std::string tc2uuid (TCUuid& uuid)
{
  char s[TC_UUID_STRING_BYTES];
  tc_uuid_to_buf (uuid, s);
  std::string str;
  str.assign (s, TC_UUID_STRING_BYTES);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
}
