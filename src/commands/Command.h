////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_COMMAND
#define INCLUDED_COMMAND

#include <map>
#include <vector>
#include <string>
#include <Task.h>

class Command
{
public:
  enum class Category
  {
    unassigned,
    // In presentation ("usefulness") order: frequently-used categories first.
    metadata,
    report,
    operation,
    context,
    graphs,
    config,
    migration,
    misc,
    internal,
    UNDOCUMENTED,
    // Whenever you extend this enum, update categoryNames.
  };

  Command ();
  virtual ~Command ();

  static void factory (std::map <std::string, Command*>&);

  std::string keyword () const;
  std::string usage () const;
  std::string description () const;
  bool read_only () const;
  bool displays_id () const;
  bool needs_gc () const;
  bool uses_context () const;
  bool accepts_filter () const;
  bool accepts_modifications () const;
  bool accepts_miscellaneous () const;
  Category category () const;
  virtual int execute (std::string&) = 0;

protected:
  bool permission (const std::string&, unsigned int);
  static const std::map <Command::Category, std::string> categoryNames;

protected:
  std::string _keyword;
  std::string _usage;
  std::string _description;
  bool        _read_only;
  bool        _displays_id;
  bool        _needs_confirm;
  bool        _needs_gc;
  bool        _uses_context;
  bool        _accepts_filter;
  bool        _accepts_modifications;
  bool        _accepts_miscellaneous;
  Category    _category;

  // Permission support
  bool        _permission_quit;
  bool        _permission_all;
  bool        _first_iteration;
};

#endif
////////////////////////////////////////////////////////////////////////////////
