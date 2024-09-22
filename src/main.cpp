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

#include <Context.h>
#include <rust/cxx.h>
#include <signal.h>

#include <cstring>
#include <iostream>
#include <new>
#include <regex>

////////////////////////////////////////////////////////////////////////////////
int main(int argc, const char** argv) {
  int status{0};

  // Ignore SIGPIPE from writes to network sockets after the remote end has hung
  // up. Rust code expects this, and the Rust runtime ignores this signal at startup.
  signal(SIGPIPE, SIG_IGN);

  Context globalContext;
  Context::setContext(&globalContext);

  // Lightweight version checking that doesn't require initialization or any I/O.
  if (argc == 2 && !strcmp(argv[1], "--version")) {
    std::cout << VERSION << "\n";
  } else {
    try {
      status = Context::getContext().initialize(argc, argv);
      if (status == 0) status = Context::getContext().run();
    }

    catch (const std::string& error) {
      std::cerr << error << "\n";
      status = -1;
    }

    catch (rust::Error& err) {
      std::cerr << err.what() << "\n";
      status = -1;
    }

    catch (std::bad_alloc& error) {
      std::cerr << "Error: Memory allocation failed: " << error.what() << "\n";
      status = -3;
    }

    catch (const std::regex_error& e) {
      std::cout << "regex_error caught: " << e.what() << '\n';
    }

    catch (...) {
      std::cerr << "Unknown error. Please report.\n";
      status = -2;
    }
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
