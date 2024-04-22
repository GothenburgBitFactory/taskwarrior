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

#include <Version.h>
#include <cmake.h>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
Version::Version(std::string version) {
  std::vector<int> parts;
  std::string part;
  std::istringstream input(version);

  while (std::getline(input, part, '.')) {
    int value;
    // Try converting string to integer
    if (std::stringstream(part) >> value && value >= 0) {
      parts.push_back(value);
    } else {
      return;
    }
  }

  if (parts.size() != 3) {
    return;
  }

  major = parts[0];
  minor = parts[1];
  patch = parts[2];
}

////////////////////////////////////////////////////////////////////////////////
Version Version::Current() { return Version(PACKAGE_VERSION); }

////////////////////////////////////////////////////////////////////////////////
bool Version::is_valid() const { return major >= 0; }

////////////////////////////////////////////////////////////////////////////////
bool Version::operator<(const Version &other) const {
  return std::tie(major, minor, patch) <
         std::tie(other.major, other.minor, other.patch);
}

////////////////////////////////////////////////////////////////////////////////
bool Version::operator<=(const Version &other) const {
  return std::tie(major, minor, patch) <=
         std::tie(other.major, other.minor, other.patch);
}

////////////////////////////////////////////////////////////////////////////////
bool Version::operator>(const Version &other) const {
  return std::tie(major, minor, patch) >
         std::tie(other.major, other.minor, other.patch);
}

////////////////////////////////////////////////////////////////////////////////
bool Version::operator>=(const Version &other) const {
  return std::tie(major, minor, patch) >=
         std::tie(other.major, other.minor, other.patch);
}

////////////////////////////////////////////////////////////////////////////////
bool Version::operator==(const Version &other) const {
  return std::tie(major, minor, patch) ==
         std::tie(other.major, other.minor, other.patch);
}

////////////////////////////////////////////////////////////////////////////////
bool Version::operator!=(const Version &other) const {
  return std::tie(major, minor, patch) !=
         std::tie(other.major, other.minor, other.patch);
}

////////////////////////////////////////////////////////////////////////////////
Version::operator std::string() const {
  std::ostringstream output;
  if (is_valid()) {
    output << major << '.' << minor << '.' << patch;
  } else {
    output << "(invalid version)";
  }
  return output.str();
}

////////////////////////////////////////////////////////////////////////////////
std::ostream &operator<<(std::ostream &os, const Version &version) {
  os << std::string(version);
  return os;
}
