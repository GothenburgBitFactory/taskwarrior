#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 43;

# Create the rc file.
if (open my $fh, '>', 'dep.rc')
{
  print $fh "data.location=.\n";
  print $fh "dependency.confirmation=yes\n";
  print $fh "report.depreport.columns=id,depends,description\n";
  print $fh "report.depreport.labels=ID,Depends,Description\n";
  print $fh "report.depreport.filter=status:pending\n";
  print $fh "report.depreport.sort=depends+\n";
  close $fh;

  # [1]
  ok (-r 'dep.rc', 'Created dep.rc');
}

qx{../src/task rc:dep.rc add One};
qx{../src/task rc:dep.rc add Two};

# [2]
my $output = qx{../src/task rc:dep.rc 1 modify dep:-2};
like ($output, qr/Modified 0 tasks\./, 'dependencies - remove nonexistent dependency');

# [3]
$output = qx{../src/task rc:dep.rc 1 modify dep:99};
like ($output, qr/Could not create a dependency on task 99 - not found\./, 'dependencies - add dependency for nonexistent task');

# [4]
$output = qx{../src/task rc:dep.rc 99 modify dep:1};
like ($output, qr/No tasks specified\./, 'dependencies - add dependency to nonexistent task');

# [5,6] t 1 dep:2; t info 1 => blocked by 2
$output = qx{../src/task rc:dep.rc 1 modify dep:2; ../src/task rc:dep.rc info 1};
like ($output, qr/This task blocked by\s+2 Two\nUUID/, 'dependencies - trivial blocked');
unlike ($output, qr/This task is blocking\n/,          'dependencies - trivial blocked');

# [7,8] t info 2 => blocking 1
$output = qx{../src/task rc:dep.rc info 2};
unlike ($output, qr/This task blocked by/,              'dependencies - trivial blocking');
like ($output, qr/This task is blocking\s+1 One\nUUID/, 'dependencies - trivial blocking');

# [9] t 1 dep:2 (again)
$output = qx{../src/task rc:dep.rc 1 modify dep:2};
like ($output, qr/Task 1 already depends on task 2\./, 'dependencies - add already existing dependency');

# [10,11] t 1 dep:1 => error
$output = qx{../src/task rc:dep.rc 1 modify dep:1};
like   ($output, qr/A task cannot be dependent on itself\./, 'dependencies - cannot depend on self');
unlike ($output, qr/Modified 1 task\./,                      'dependencies - cannot depend on self');

# [12,13] t 1 dep:2; t 2 dep:1 => error
$output = qx{../src/task rc:dep.rc 2 modify dep:1};
like   ($output, qr/Circular dependency detected and disallowed\./, 'dependencies - trivial circular');
unlike ($output, qr/Modified 1 task\./,                             'dependencies - trivial circular');

# [14,15] t 1 dep:2; t 2 dep:3; t 1 dep:3 => not circular
qx{../src/task rc:dep.rc 1 modify dep:2};
qx{../src/task rc:dep.rc add Three};
qx{../src/task rc:dep.rc 2 modify dep:3};
$output = qx{../src/task rc:dep.rc 1 modify dep:3};
unlike ($output, qr/Circular dependency detected and disallowed\./, 'dependencies - diamond, non-circular');
like   ($output, qr/Modified 1 task\./,                             'dependencies - diamond, non-circular');

# [16]
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../src/task rc:dep.rc add One};
qx{../src/task rc:dep.rc add Two};
qx{../src/task rc:dep.rc add Three};
qx{../src/task rc:dep.rc add Four};
qx{../src/task rc:dep.rc add Five};

qx{../src/task rc:dep.rc 5 modify dep:4; ../src/task rc:dep.rc 4 modify dep:3; ../src/task rc:dep.rc 3 modify dep:2; ../src/task rc:dep.rc 2 modify dep:1};

# [17,18] 5 dep 4 dep 3 dep 2 dep 1 dep 5 => error
$output = qx{../src/task rc:dep.rc 1 modify dep:5};
like   ($output, qr/Circular dependency detected and disallowed\./, 'dependencies - nontrivial circular');
unlike ($output, qr/Modified 1 task\./,                             'dependencies - nontrivial circular');

# [19]
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../src/task rc:dep.rc add One};
qx{../src/task rc:dep.rc add Two};
qx{../src/task rc:dep.rc add Three};
qx{../src/task rc:dep.rc add Four};
qx{../src/task rc:dep.rc add Five};
qx{../src/task rc:dep.rc add Six recurring due:tomorrow recur:daily};

# [20]
qx{../src/task rc:dep.rc ls}; # To force handleRecurrence call.
$output = qx{../src/task rc:dep.rc 6 modify dep:5};
like ($output, qr/Modified \d+ task/, 'dependencies - recurring task depending on another task');

# [21]
$output = qx{../src/task rc:dep.rc 4 modify dep:5};
like ($output, qr/Modified \d+ task/, 'dependencies - task depending on recurring task');

# [22] t 1 dep:2,3,4; t 1 dep:-2,-4,5; t info 1 => blocked by 3,5
$output = qx{../src/task rc:dep.rc 1 modify dep:2,3,4; ../src/task rc:dep.rc 1 modify dep:-2,-4,5; ../src/task rc:dep.rc info 1};
like ($output, qr/This task blocked by\s+3 Three\n\s+5 Five\nUUID/, 'dependencies - multiple dependencies modified');

# [23,24]
$output = qx{../src/task rc:dep.rc 3,5 do; ../src/task rc:dep.rc info 1};
unlike ($output, qr/This task blocked by/, 'dependencies - task info reflects completed dependencies');
unlike ($output, qr/This task is blocking/, 'dependencies - task info reflects completed dependencies');

# [25]
$output = qx{../src/task rc:dep.rc depreport};
like ($output, qr/\s1\s+One\s+/, 'dependencies - depends report column reflects completed dependencies');

# [26]
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../src/task rc:dep.rc add One};
qx{../src/task rc:dep.rc add Two};
qx{../src/task rc:dep.rc add Three};
qx{../src/task rc:dep.rc add Four};

qx{../src/task rc:dep.rc 1 modify dep:3,4};
qx{../src/task rc:dep.rc 2 do};

# [27]
$output = qx{../src/task rc:dep.rc depreport};
like ($output, qr/\s1\s+2 3\s+One\s+/, 'dependencies - depends report column reflects changed IDs');

# [28]
qx{../src/task rc:dep.rc 3 do};
$output = qx{../src/task rc:dep.rc depreport};
like ($output, qr/\s1\s+2\s+One\s+/, 'dependencies - depends report column reflects completed dependencies');

# [29]
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../src/task rc:dep.rc add One};
qx{../src/task rc:dep.rc add Two};
qx{../src/task rc:dep.rc add Three};
qx{../src/task rc:dep.rc add Four};

qx{../src/task rc:dep.rc 2 modify dep:1; ../src/task rc:dep.rc 3 modify dep:2; ../src/task rc:dep.rc 4 modify dep:3};

# [30,31]
$output = qx{echo '-- y' | ../src/task rc:dep.rc 2 do};
like ($output, qr/fixed/, 'dependencies - user prompted to fix broken chain after completing a blocked task');
like ($output, qr/is blocked by/, 'dependencies - user nagged for completing a blocked task');

# [32]
$output = qx{echo '-- y' | ../src/task rc:dep.rc 1 do};
unlike ($output, qr/fixed/, 'dependencies - user not prompted to fix broken chain when the head of the chain is marked as complete');

# [33]
$output = qx{echo '-- y' | ../src/task rc:dep.rc 4 del};
unlike ($output, qr/fixed/, 'dependencies - user not prompted to fix broken chain when the tail of the chain is deleted');

# [34]
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data for a fresh start');

qx{../src/task rc:dep.rc add One};
qx{../src/task rc:dep.rc add Two};
qx{../src/task rc:dep.rc add Three};
qx{../src/task rc:dep.rc add Four};
qx{../src/task rc:dep.rc add Five};

qx{../src/task rc:dep.rc 2 modify dep:1};
qx{../src/task rc:dep.rc 3 modify dep:2};
qx{../src/task rc:dep.rc 4 modify dep:3};
qx{../src/task rc:dep.rc 5 modify dep:4};

# [35]
qx{echo '-- y' | ../src/task rc:dep.rc 2 do};
$output = qx{../src/task rc:dep.rc depreport};
like ($output, qr/\s1\s+One\s*\n\s2\s+1\s+Three\s*\n\s3\s+2\s+Four\s*\n\s4\s+3\s+Five/, 'dependencies - fixed chain after completing a blocked task');

# [36]
qx{echo "-- Y\nY\n" | ../src/task rc:dep.rc 2 del};
$output = qx{../src/task rc:dep.rc depreport};
like ($output, qr/\s1\s+One\s*\n\s2\s+1\s+Four\s*\n\s3\s+2\s+Five/, 'dependencies - fixed chain after deleting a blocked task');

# [37]
qx{../src/task rc:dep.rc 2 modify dep:-1};
$output = qx{../src/task rc:dep.rc depreport};
like ($output, qr/\s1\s+One\s*\n\s2\s+Four\s*\n\s3\s+2\s+Five/, 'dependencies - chain should not be automatically repaired after manually removing a dependency');

# TODO - test dependency.confirmation config variable
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

unlink 'backlog.data';
ok (!-r 'backlog.data', 'Removed backlog.data');

unlink 'synch.key';
ok (!-r 'synch.key', 'Removed synch.key');

unlink 'dep.rc';
ok (!-r 'dep.rc', 'Removed dep.rc');

exit 0;

