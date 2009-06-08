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
#include "task.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest test (34);

  test.is ((int)T2::textToStatus ("pending"),   (int)T2::pending,   "textToStatus pending");
  test.is ((int)T2::textToStatus ("completed"), (int)T2::completed, "textToStatus completed");
  test.is ((int)T2::textToStatus ("deleted"),   (int)T2::deleted,   "textToStatus deleted");
  test.is ((int)T2::textToStatus ("recurring"), (int)T2::recurring, "textToStatus recurring");

  test.is (T2::statusToText (T2::pending),   "pending",   "statusToText pending");
  test.is (T2::statusToText (T2::completed), "completed", "statusToText completed");
  test.is (T2::statusToText (T2::deleted),   "deleted",   "statusToText deleted");
  test.is (T2::statusToText (T2::recurring), "recurring", "statusToText recurring");

  // Round-trip testing.
  T2 t3;
  std::string before = t3.composeF4 ();
  t3.parse (before);
  std::string after = t3.composeF4 ();
  t3.parse (after);
  after = t3.composeF4 ();
  t3.parse (after);
  after = t3.composeF4 ();
  test.is (before, after, "T2::composeF4 -> parse round trip 4 iterations");

  // Legacy Format 1
  //   [tags] [attributes] description\n
  //   X [tags] [attributes] description\n
  std::string sample = "[tag1 tag2] [att1:value1 att2:value2] Description";
  sample = "X "
           "[one two] "
           "[att1:value1 att2:value2] "
           "Description";
  bool good = true;
  try { T2 ff1 (sample); } catch (...) { good = false; }
  test.notok (good, "Support for ff1 removed");

  // Legacy Format 2
  //   uuid status [tags] [attributes] description\n
  sample = "00000000-0000-0000-0000-000000000000 "
           "- "
           "[tag1 tag2] "
           "[att1:value1 att2:value2] "
           "Description";
  T2 ff2 (sample);
  std::string value = ff2.get ("uuid");
  test.is (value, "00000000-0000-0000-0000-000000000000", "ff2 uuid");
  value = ff2.get ("status");
  test.is (value, "pending", "ff2 status");
  test.ok (ff2.hasTag ("tag1"), "ff2 tag1");
  test.ok (ff2.hasTag ("tag2"), "ff2 tag2");
  test.is (ff2.getTagCount (), 2, "ff2 # tags");
  value = ff2.get ("att1");
  test.is (value, "value1", "ff2 att1");
  value = ff2.get ("att2");
  test.is (value, "value2", "ff2 att2");
  value = ff2.get ("description");
  test.is (value, "Description", "ff2 description");

  // Legacy Format 3
  //   uuid status [tags] [attributes] [annotations] description\n
  sample = "00000000-0000-0000-0000-000000000000 "
           "- "
           "[tag1 tag2] "
           "[att1:value1 att2:value2] "
           "[123:ann1 456:ann2] Description";
  T2 ff3 (sample);
  value = ff2.get ("uuid");
  test.is (value, "00000000-0000-0000-0000-000000000000", "ff3 uuid");
  value = ff2.get ("status");
  test.is (value, "pending", "ff3 status");
  test.ok (ff2.hasTag ("tag1"), "ff3 tag1");
  test.ok (ff2.hasTag ("tag2"), "ff3 tag2");
  test.is (ff2.getTagCount (), 2, "ff3 # tags");
  value = ff3.get ("att1");
  test.is (value, "value1", "ff3 att1");
  value = ff3.get ("att2");
  test.is (value, "value2", "ff3 att2");
  value = ff3.get ("description");
  test.is (value, "Description", "ff3 description");

  // Current Format 4
  //   [name:"value" ...]\n
  sample = "["
           "uuid:\"00000000-0000-0000-0000-000000000000\" "
           "status:\"P\" "
           "tags:\"tag1&commaltag2\" "
           "att1:\"value1\" "
           "att2:\"value2\" "
           "description:\"Description\""
           "]";
  T2 ff4 (sample);
  value = ff2.get ("uuid");
  test.is (value, "00000000-0000-0000-0000-000000000000", "ff4 uuid");
  value = ff2.get ("status");
  test.is (value, "pending", "ff4 status");
  test.ok (ff2.hasTag ("tag1"), "ff4 tag1");
  test.ok (ff2.hasTag ("tag2"), "ff4 tag2");
  test.is (ff2.getTagCount (), 2, "ff4 # tags");
  value = ff4.get ("att1");
  test.is (value, "value1", "ff4 att1");
  value = ff4.get ("att2");
  test.is (value, "value2", "ff4 att2");
  value = ff4.get ("description");
  test.is (value, "Description", "ff4 description");

/*

T2::composeCSV
T2::id
T2::*Status
T2::*Tag*
T2::*Annotation*

*/

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

