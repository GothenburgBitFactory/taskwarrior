////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2024, Dustin Mitchell.
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

#ifndef INCLUDED_VERSION
#define INCLUDED_VERSION

#include <string>

// A utility class for handling Taskwarrior versions.
class Version {
public:
  // Parse a version from a string. This must be of the format
  // digits.digits.digits.
  explicit Version(std::string version);

  // Create an invalid version.
  Version() = default;

  Version(const Version &other) = default;
  Version(Version &&other) = default;
  Version &operator=(const Version &) = default;
  Version &operator=(Version &&) = default;

  // Return a version representing the release being built.
  static Version Current();

  bool is_valid() const;

  // Compare versions.
  bool operator<(const Version &) const;
  bool operator<=(const Version &) const;
  bool operator>(const Version &) const;
  bool operator>=(const Version &) const;
  bool operator==(const Version &) const;
  bool operator!=(const Version &) const;

  // Convert back to a string.
  operator std::string() const;

  friend std::ostream& operator<<(std::ostream& os, const Version& version);

private:
  int major = -1;
  int minor = -1;
  int patch = -1;
};

#endif
////////////////////////////////////////////////////////////////////////////////
