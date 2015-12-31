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

#include <cmake.h>
#include <CmdExport.h>
#include <Context.h>
#include <Filter.h>
#include <main.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdExport::CmdExport ()
{
  _keyword               = "export";
  _usage                 = "task <filter> export";
  _description           = STRING_CMD_EXPORT_USAGE;
  _read_only             = true;
  _displays_id           = true;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::migration;
}

////////////////////////////////////////////////////////////////////////////////
int CmdExport::execute (std::string& output)
{
  int rc = 0;

  // Make sure reccurent tasks are generated.
  handleRecurrence ();

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Export == render.
  context.timer_render.start ();

  // Obey 'limit:N'.
  int rows = 0;
  int lines = 0;
  context.getLimits (rows, lines);
  int limit = (rows > lines ? rows : lines);

  // Is output contained within a JSON array?
  bool json_array = context.config.getBoolean ("json.array");

  // Compose output.
  if (json_array)
    output += "[\n";

  int counter = 0;
  for (auto& task : filtered)
  {
    if (counter)
    {
      if (json_array)
        output += ",";
      output += "\n";
    }

    output += task.composeJSON (true);

    ++counter;
    if (limit && counter >= limit)
      break;
  }

  if (filtered.size ())
    output += "\n";

  if (json_array)
    output += "]\n";

  context.timer_render.stop ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
