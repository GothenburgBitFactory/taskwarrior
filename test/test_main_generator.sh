#! /bin/bash
#///////////////////////////////////////////////////////////////////////////////
#/
#/ Copyright 2024 - 2024, Göteborg Bit Factory.
#/
#/ Permission is hereby granted, free of charge, to any person obtaining a copy
#/ of this software and associated documentation files (the "Software"), to deal
#/ in the Software without restriction, including without limitation the rights
#/ to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#/ copies of the Software, and to permit persons to whom the Software is
#/ furnished to do so, subject to the following conditions:
#/
#/ The above copyright notice and this permission notice shall be included
#/ in all copies or substantial portions of the Software.
#/
#/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#/ OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#/ FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#/ THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#/ LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#/ OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#/ SOFTWARE.
#/
#/ https://www.opensource.org/licenses/mit-license.php
#/
#///////////////////////////////////////////////////////////////////////////////

cat <<'EOF'
#include <iostream>
#include <cassert>
int main(int argc, char** argv) {
  // Expect to be called as `test_runner <test-name>`.
  assert(argc == 2);
  std::string test_name = argv[1];
EOF
for test_file in "${@}"; do
  # same substitution as made in CMakeLists.txt
  test_name=${test_file//[^a-zA-Z0-9]/_}
  cat <<EOF
  if (test_name == "${test_name}") {
    int ${test_name}();  // forward declaration
    ${test_name}();
  }
EOF
done
cat <<'EOF'
  std::cerr << "No such test suite " << test_name << "\n";
  return 1;
}
EOF
