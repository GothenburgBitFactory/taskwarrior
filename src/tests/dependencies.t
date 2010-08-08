#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham.
## All rights reserved.
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 2 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the
##
##     Free Software Foundation, Inc.,
##     51 Franklin Street, Fifth Floor,
##     Boston, MA
##     02110-1301
##     USA
##
################################################################################

use strict;
use warnings;
use Test::More tests => 32;

# Create the rc file.
if (open my $fh, '>', 'dep.rc')
{
  print $fh "data.location=.\n";
  print $fh "dependency.confirm=yes\n";
  print $fh "report.depreport.columns=id,depends,description\n";
  print $fh "report.depreport.labels=ID,Depends,Description\n";
  print $fh "report.depreport.filter=status:pending\n";
  print $fh "report.depreport.sort=depends+\n";
  print $fh "nag=NAG";
  close $fh;

  ok (-r 'dep.rc', 'Created dep.rc');
}

qx{../task rc:dep.rc add One};
qx{../task rc:dep.rc add Two};

my $output = qx{../task rc:dep.rc 1 dep:-2};
like ($output, qr/Modified 0 tasks\./, 'dependencies - remove nonexistent dependency');

$output = qx{../task rc:dep.rc 1 dep:99};
like ($output, qr/Could not create a dependency on task 99 - not found\./, 'dependencies - add dependency for nonexistent task');

$output = qx{../task rc:dep.rc 99 dep:1};
like ($output, qr/Task 99 not found\./, 'dependencies - add dependency to nonexistent task');

# t 1 dep:2; t info 1 => blocked by 2
$output = qx{../task rc:dep.rc 1 dep:2; ../task rc:dep.rc info 1};
like ($output, qr/This task blocked by\s+2 Two\nThis task is blocking/, 'dependencies - trivial blocked');

# t info 2 => blocking 1
$output = qx{../task rc:dep.rc info 2};
like ($output, qr/This task is blocking\s+1 One\nUUID/, 'dependencies - trivial blocking');

# t 1 dep:2 (again)
$output = qx{../task rc:dep.rc 1 dep:2};
like ($output, qr/Modified 0 tasks\./, 'dependencies - add already existing dependency');

# t 1 dep:2; t 2 dep:1 => error
$output = qx{../task rc:dep.rc 2 dep:1};
unlike ($output, qr/Modified 1 task\./, 'dependencies - trivial circular');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../task rc:dep.rc add One};
qx{../task rc:dep.rc add Two};
qx{../task rc:dep.rc add Three};
qx{../task rc:dep.rc add Four};
qx{../task rc:dep.rc add Five};

qx{../task rc:dep.rc 5 dep:4; ../task rc:dep.rc 4 dep:3; ../task rc:dep.rc 3 dep:2; ../task rc:dep.rc 2 dep:1};

# 5 dep 4 dep 3 dep 2 dep 1 dep 5 => error
$output = qx{../task rc:dep.rc 1 dep:5};
unlike ($output, qr/Modified 1 task\./, 'dependencies - nontrivial circular');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../task rc:dep.rc add One};
qx{../task rc:dep.rc add Two};
qx{../task rc:dep.rc add Three};
qx{../task rc:dep.rc add Four};
qx{../task rc:dep.rc add Five};
qx{../task rc:dep.rc add Six recurring due:tomorrow recur:daily};

$output = qx{../task rc:dep.rc 6 dep:5};
unlike ($output,qr/Modified \d+ task/, 'dependencies - recurring task depending on another task');

$output = qx{../task rc:dep.rc 5 dep:6};
like ($output,qr/Modified \d+ task/, 'dependencies - task depending on recurring task');

# t 1 dep:2,3,4; t 1 dep:-2,-4,5; t info 1 => blocked by 3,5
$output = qx{../task rc:dep.rc 1 dep:2,3,4; ../task rc:dep.rc 1 dep:-2,-4,5; ../task rc:dep.rc info 1};
like ($output, qr/This task blocked by\s+3 Three\n\s+5 Five\nThis task is blocking/, 'dependencies - multiple dependencies modified');

$output = qx{../task rc:dep.rc do 3,5; ../task rc:dep.rc info 1};
like ($output, qr/This task blocked by\nThis task is blocking/, 'dependencies - task info reflects completed dependencies');

$output = qx{../task rc:dep.rc depreport};
like ($output, qr/\s1\s+One\s+/, 'dependencies - depends report column reflects completed dependencies');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../task rc:dep.rc add One};
qx{../task rc:dep.rc add Two};
qx{../task rc:dep.rc add Three};
qx{../task rc:dep.rc add Four};

qx{../task rc:dep.rc 1 dep:3,4};
qx{../task rc:dep.rc do 2};

$output = qx{../task rc:dep.rc depreport};
like ($output, qr/\s1\s+2, 3\s+One\s+/, 'dependencies - depends report column reflects changed IDs');

$output = qx{../task rc:dep.rc depreport};
like ($output, qr/\s1\s+One\s+/, 'dependencies - depends report column reflects completed dependencies');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../task rc:dep.rc add One};
qx{../task rc:dep.rc add Two};
qx{../task rc:dep.rc add Three};
qx{../task rc:dep.rc add Four};

qx{../task rc:dep.rc 2 dep:1; ../task rc:dep.rc 3 dep:2; ../task rc:dep.rc 4 dep:3};

$output = qx{echo y | ../task rc:dep.rc do 2};
like ($output, qr/fixed/, 'dependencies - user prompted to fix broken chain after completing a blocked task');

like ($output, qr/NAG/, 'dependencies - user nagged for completing a blocked task');

$output = qx{echo y | ../task rc:dep.rc do 1};
unlike ($output, qr/fixed/, 'dependencies - user not prompted to fix broken chain when the head of the chain is marked as complete');

$output = qx{echo y | ../task rc:dep.rc del 4};
unlike ($output, qr/fixed/, 'dependencies - user not prompted to fix broken chain when the tail of the chain is deleted');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../task rc:dep.rc add One};
qx{../task rc:dep.rc add Two};
qx{../task rc:dep.rc add Three};
qx{../task rc:dep.rc add Four};
qx{../task rc:dep.rc add Five};

qx{../task rc:dep.rc 2 dep:1};
qx{../task rc:dep.rc 3 dep:2};
qx{../task rc:dep.rc 4 dep:3};
qx{../task rc:dep.rc 5 dep:4};

qx{echo y | ../task rc:dep.rc do 2};
$output = qx{../task rc:dep.rc depreport};

like ($output, qr/\s1\s+One\s*\n\s2\s+1\s+Three\s*\n\s3\s+2\s+Four\s*\n\s4\s+3\s+Five/, 'dependencies - fixed chain after completing a blocked task');

# TODO TODO TODO - Need to echo Y Y (once for delete confirmation, again for repair prompt)
qx{echo y | ../task rc:dep.rc del 2}; 
$output = qx{../task rc:dep.rc depreport};
like ($output, qr/\s1\s+One\s*\n\s2\s+1\s+Four\s*\n\s3\s+2\s+Five/, 'dependencies - fixed chain after deleting a blocked task');

qx{../task rc:dep.rc 2 dep:-1}; 
$output = qx{../task rc:dep.rc depreport};
like ($output, qr/\s1\s+One\s*\n\s2\s+Four\s*\n\s3\s+2\s+Five/, 'dependencies - chain should not be automatically repaired after manually removing a dependency');

# TODO - test dependency.confirm config variable
# TODO - test undo on backing out chain gap repair
# TODO - test undo on backing out choice to not perform chain gap repair
# TODO - test blocked task completion nag
# TODO - test depend.any and depend.none report filters

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'dep.rc';
ok (!-r 'dep.rc', 'Removed dep.rc');

exit 0;

