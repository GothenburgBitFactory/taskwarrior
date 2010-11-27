////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
  UnitTest t (86);

  // Without Context::initialize, there is no set of defaults loaded into
  // Context::Config.
  context.initialize ();
  context.config.set ("report.foo.columns", "id");

  Cmd cmd;
  cmd.command = "active";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand active");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand active");

  cmd.command = "calendar";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand calendar");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand calendar");

  cmd.command = "colors";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand colors");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand colors");

  cmd.command = "completed";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand completed");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand completed");

  cmd.command = "export.csv";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand export.csv");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand export.csv");

  cmd.command = "export.ical";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand export.ical");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand export.ical");

  cmd.command = "help";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand help");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand help");

  cmd.command = "history.monthly";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand history.monthly");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand history.monthly");

  cmd.command = "history.annual";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand history.annual");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand history.annual");

  cmd.command = "ghistory.monthly";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand ghistory.monthly");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand ghistory.monthly");

  cmd.command = "ghistory.annual";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand ghistory.annual");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand ghistory.annual");

  cmd.command = "info";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand info");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand info");

  cmd.command = "next";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand next");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand next");

  cmd.command = "overdue";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand overdue");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand overdue");

  cmd.command = "projects";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand projects");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand projects");

  cmd.command = "stats";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand stats");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand stats");

  cmd.command = "summary";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand summary");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand summary");

  cmd.command = "tags";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand tags");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand tags");

  cmd.command = "timesheet";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand timesheet");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand timesheet");

  cmd.command = "version";
  t.ok    (cmd.isReadOnlyCommand (), "isReadOnlyCommand version");
  t.notok (cmd.isWriteCommand    (), "not isWriteCommand version");

  cmd.command = "_projects";
  t.ok    (cmd.isReadOnlyCommand (), "not isReadOnlyCommand _projects");
  t.notok (cmd.isWriteCommand    (), "isWriteCommand _projects");

  cmd.command = "_tags";
  t.ok    (cmd.isReadOnlyCommand (), "not isReadOnlyCommand _tags");
  t.notok (cmd.isWriteCommand    (), "isWriteCommand _tags");

  cmd.command = "add";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand add");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand add");

  cmd.command = "log";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand log");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand log");

  cmd.command = "append";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand append");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand append");

  cmd.command = "annotate";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand annotate");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand annotate");

  cmd.command = "delete";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand delete");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand delete");

  cmd.command = "done";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand done");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand done");

  cmd.command = "duplicate";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand duplicate");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand duplicate");

  cmd.command = "edit";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand edit");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand edit");

  cmd.command = "import";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand import");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand import");

  cmd.command = "start";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand start");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand start");

  cmd.command = "stop";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand stop");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand stop");

  cmd.command = "undo";
  t.notok (cmd.isReadOnlyCommand (), "not isReadOnlyCommand undo");
  t.ok    (cmd.isWriteCommand    (), "isWriteCommand undo");

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
