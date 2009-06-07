////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////
#include <Context.h>
#include <Cmd.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (18);

  context.config.set ("report.foo.columns", "id");

  Cmd cmd;
  t.ok (cmd.valid ("annotate"), "Cmd::valid annotate");
  t.ok (cmd.valid ("annotat"),  "Cmd::valid annotat");
  t.ok (cmd.valid ("annota"),   "Cmd::valid annota");
  t.ok (cmd.valid ("annot"),    "Cmd::valid annot");
  t.ok (cmd.valid ("anno"),     "Cmd::valid anno");
  t.ok (cmd.valid ("ann"),      "Cmd::valid ann");
  t.ok (cmd.valid ("an"),       "Cmd::valid an");

  t.ok (cmd.valid ("ANNOTATE"), "Cmd::valid ANNOTATE");
  t.ok (cmd.valid ("ANNOTAT"),  "Cmd::valid ANNOTAT");
  t.ok (cmd.valid ("ANNOTA"),   "Cmd::valid ANNOTA");
  t.ok (cmd.valid ("ANNOT"),    "Cmd::valid ANNOT");
  t.ok (cmd.valid ("ANNO"),     "Cmd::valid ANNO");
  t.ok (cmd.valid ("ANN"),      "Cmd::valid ANN");
  t.ok (cmd.valid ("AN"),       "Cmd::valid AN");

  t.ok    (cmd.validCustom ("foo"), "Cmd::validCustom foo");
  t.notok (cmd.validCustom ("bar"), "Cmd::validCustom bar -> fail");

  bool good = true;
  try { cmd.parse ("a"); } catch (...) { good = false; }
  t.notok (good, "Cmd::parse a -> fail");

  good = true;
  try { cmd.parse ("add"); } catch (...) { good = false; }
  t.ok (good, "Cmd::parse add");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
