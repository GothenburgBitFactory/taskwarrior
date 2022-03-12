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

#ifndef INCLUDED_TC_UTIL
#define INCLUDED_TC_UTIL

#include <string>
#include "tc/ffi.h"

namespace tc {
  // convert a std::string into a TCString, copying the contained data
  tc::ffi::TCString string2tc(const std::string&);

  // convert a TCString into a std::string, leaving the TCString as-is
  std::string tc2string_clone(const tc::ffi::TCString&);

  // convert a TCString into a std::string, freeing the TCString
  std::string tc2string(tc::ffi::TCString&);

  // convert a TCUuid into a std::string
  std::string tc2uuid(tc::ffi::TCUuid&);

  // parse a std::string into a TCUuid (throwing if parse fails)
  tc::ffi::TCUuid uuid2tc(const std::string&);
}

#endif
////////////////////////////////////////////////////////////////////////////////
