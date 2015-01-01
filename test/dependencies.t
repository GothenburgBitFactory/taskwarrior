#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

use strict;
use warnings;
use Test::More tests => 41;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

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
}

qx{../src/task rc:dep.rc add One 2>&1};
qx{../src/task rc:dep.rc add Two 2>&1};

# [2]
my $output = qx{../src/task rc:dep.rc 1 modify dep:-2 2>&1 >/dev/null};
like ($output, qr/Could not delete a dependency on task 2 - not found\./, 'dependencies - remove nonexistent dependency');

# [3]
$output = qx{../src/task rc:dep.rc 1 modify dep:99 2>&1 >/dev/null};
like ($output, qr/Could not create a dependency on task 99 - not found\./, 'dependencies - add dependency for nonexistent task');

# [4]
$output = qx{../src/task rc:dep.rc 99 modify dep:1 2>&1 >/dev/null};
like ($output, qr/No tasks specified\./, 'dependencies - add dependency to nonexistent task');

# [5,6] t 1 dep:2; t info 1 => blocked by 2
$output = qx{../src/task rc:dep.rc 1 modify dep:2 2>&1; ../src/task rc:dep.rc info 1 2>&1};
like ($output, qr/This task blocked by\s+2 Two\n/, 'dependencies - trivial blocked');
unlike ($output, qr/This task is blocking\n/,      'dependencies - trivial blocked');

# [7,8] t info 2 => blocking 1
$output = qx{../src/task rc:dep.rc info 2 2>&1};
unlike ($output, qr/This task blocked by/,          'dependencies - trivial blocking');
like ($output, qr/This task is blocking\s+1 One\n/, 'dependencies - trivial blocking');

# [9] t 1 dep:2 (again)
$output = qx{../src/task rc:dep.rc 1 modify dep:2 2>&1 >/dev/null};
like ($output, qr/Task 1 already depends on task 2\./, 'dependencies - add already existing dependency');

# [10,11] t 1 dep:1 => error
$output = qx{../src/task rc:dep.rc 1 modify dep:1 2>&1};
like   ($output, qr/A task cannot be dependent on itself\./, 'dependencies - cannot depend on self');
unlike ($output, qr/Modified 1 task\./,                      'dependencies - cannot depend on self');

# [12,13] t 1 dep:2; t 2 dep:1 => error
$output = qx{../src/task rc:dep.rc 2 modify dep:1 2>&1};
like   ($output, qr/Circular dependency detected and disallowed\./, 'dependencies - trivial circular');
unlike ($output, qr/Modified 1 task\./,                             'dependencies - trivial circular');

# [14,15] t 1 dep:2; t 2 dep:3; t 1 dep:3 => not circular
qx{../src/task rc:dep.rc 1 modify dep:2 2>&1};
qx{../src/task rc:dep.rc add Three 2>&1};
qx{../src/task rc:dep.rc 2 modify dep:3 2>&1};
$output = qx{../src/task rc:dep.rc 1 modify dep:3 2>&1};
unlike ($output, qr/Circular dependency detected and disallowed\./, 'dependencies - diamond, non-circular');
like   ($output, qr/Modified 1 task\./,                             'dependencies - diamond, non-circular');

# [16]
unlink 'pending.data';

qx{../src/task rc:dep.rc add One 2>&1};
qx{../src/task rc:dep.rc add Two 2>&1};
qx{../src/task rc:dep.rc add Three 2>&1};
qx{../src/task rc:dep.rc add Four 2>&1};
qx{../src/task rc:dep.rc add Five 2>&1};

qx{../src/task rc:dep.rc 5 modify dep:4 2>&1; ../src/task rc:dep.rc 4 modify dep:3 2>&1; ../src/task rc:dep.rc 3 modify dep:2 2>&1; ../src/task rc:dep.rc 2 modify dep:1 2>&1};

# [17,18] 5 dep 4 dep 3 dep 2 dep 1 dep 5 => error
$output = qx{../src/task rc:dep.rc 1 modify dep:5 2>&1 >/dev/null};
like   ($output, qr/Circular dependency detected and disallowed\./, 'dependencies - nontrivial circular');
unlike ($output, qr/Modified 1 task\./,                             'dependencies - nontrivial circular');

# [19]
unlink 'pending.data';

qx{../src/task rc:dep.rc add One 2>&1};
qx{../src/task rc:dep.rc add Two 2>&1};
qx{../src/task rc:dep.rc add Three 2>&1};
qx{../src/task rc:dep.rc add Four 2>&1};
qx{../src/task rc:dep.rc add Five 2>&1};
qx{../src/task rc:dep.rc add Six recurring due:tomorrow recur:daily 2>&1};

# [20]
qx{../src/task rc:dep.rc ls 2>&1}; # To force handleRecurrence call.
$output = qx{echo 'y' | ../src/task rc:dep.rc 6 modify dep:5 2>&1};
like ($output, qr/Modified \d+ task/, 'dependencies - recurring task depending on another task');

# [21]
$output = qx{../src/task rc:dep.rc 4 modify dep:5 2>&1};
like ($output, qr/Modified \d+ task/, 'dependencies - task depending on recurring task');

# [22] t 1 dep:2,3,4; t 1 dep:-2,-4,5; t info 1 => blocked by 3,5
$output = qx{../src/task rc:dep.rc 1 modify dep:2,3,4 2>&1; ../src/task rc:dep.rc 1 modify dep:-2,-4,5 2>&1; ../src/task rc:dep.rc info 1 2>&1};
like ($output, qr/This task blocked by\s+3 Three\n\s+5 Five\n/, 'dependencies - multiple dependencies modified');

# [23,24]
$output = qx{../src/task rc:dep.rc 3,5 done 2>&1; ../src/task rc:dep.rc info 1 2>&1};
unlike ($output, qr/This task blocked by/, 'dependencies - task info reflects completed dependencies');
unlike ($output, qr/This task is blocking/, 'dependencies - task info reflects completed dependencies');

# [25]
$output = qx{../src/task rc:dep.rc depreport 2>&1};
like ($output, qr/\s1\s+One\s+/, 'dependencies - depends report column reflects completed dependencies');

# [26]
unlink 'pending.data';

qx{../src/task rc:dep.rc add One 2>&1};
qx{../src/task rc:dep.rc add Two 2>&1};
qx{../src/task rc:dep.rc add Three 2>&1};
qx{../src/task rc:dep.rc add Four 2>&1};

qx{../src/task rc:dep.rc 1 modify dep:3,4 2>&1};
qx{../src/task rc:dep.rc 2 done 2>&1};

# [27]
$output = qx{../src/task rc:dep.rc depreport 2>&1};
like ($output, qr/\s1\s+2 3\s+One\s+/, 'dependencies - depends report column reflects changed IDs');

# [28]
qx{../src/task rc:dep.rc 3 done 2>&1};
$output = qx{../src/task rc:dep.rc depreport 2>&1};
like ($output, qr/\s1\s+2\s+One\s+/, 'dependencies - depends report column reflects completed dependencies');

# [29]
unlink 'pending.data';

qx{../src/task rc:dep.rc add One 2>&1};
qx{../src/task rc:dep.rc add Two 2>&1};
qx{../src/task rc:dep.rc add Three 2>&1};
qx{../src/task rc:dep.rc add Four 2>&1};

qx{../src/task rc:dep.rc 2 modify dep:1 2>&1; ../src/task rc:dep.rc 3 modify dep:2 2>&1; ../src/task rc:dep.rc 4 modify dep:3 2>&1};

# [30,31]
$output = qx{echo 'y' | ../src/task rc:dep.rc 2 done 2>&1};
like ($output, qr/fixed/, 'dependencies - user prompted to fix broken chain after completing a blocked task');
like ($output, qr/is blocked by/, 'dependencies - user nagged for completing a blocked task');

# [32]
$output = qx{echo 'y' | ../src/task rc:dep.rc 1 done 2>&1};
unlike ($output, qr/fixed/, 'dependencies - user not prompted to fix broken chain when the head of the chain is marked as complete');

# [33]
$output = qx{echo 'y' | ../src/task rc:dep.rc 4 del 2>&1};
unlike ($output, qr/fixed/, 'dependencies - user not prompted to fix broken chain when the tail of the chain is deleted');

# [34]
unlink 'pending.data';

qx{../src/task rc:dep.rc add One 2>&1};
qx{../src/task rc:dep.rc add Two 2>&1};
qx{../src/task rc:dep.rc add Three 2>&1};
qx{../src/task rc:dep.rc add Four 2>&1};
qx{../src/task rc:dep.rc add Five 2>&1};

qx{../src/task rc:dep.rc 2 modify dep:1 2>&1};
qx{../src/task rc:dep.rc 3 modify dep:2 2>&1};
qx{../src/task rc:dep.rc 4 modify dep:3 2>&1};
qx{../src/task rc:dep.rc 5 modify dep:4 2>&1};

# [35]
qx{echo 'y' | ../src/task rc:dep.rc 2 done 2>&1};
$output = qx{../src/task rc:dep.rc depreport 2>&1};
like ($output, qr/\s1\s+One\s*\n\s2\s+1\s+Three\s*\n\s3\s+2\s+Four\s*\n\s4\s+3\s+Five/, 'dependencies - fixed chain after completing a blocked task');

# [36]
qx{echo "Y\nY\n" | ../src/task rc:dep.rc 2 del 2>&1};
$output = qx{../src/task rc:dep.rc depreport 2>&1};
like ($output, qr/\s1\s+One\s*\n\s2\s+1\s+Four\s*\n\s3\s+2\s+Five/, 'dependencies - fixed chain after deleting a blocked task');

# [37]
qx{../src/task rc:dep.rc 2 modify dep:-1 2>&1};
$output = qx{../src/task rc:dep.rc depreport 2>&1};
like ($output, qr/\s1\s+One\s*\n\s2\s+Four\s*\n\s3\s+2\s+Five/, 'dependencies - chain should not be automatically repaired after manually removing a dependency');

# [38]
unlink 'pending.data';

# Bug when adding a range of dependencies - 'task 3 mod dep:1-2' interprets the
# range 1-2 as the id 1

qx{../src/task rc:dep.rc add test1 2>&1};
qx{../src/task rc:dep.rc add test2 2>&1};
qx{../src/task rc:dep.rc add test3 2>&1};
qx{../src/task rc:dep.rc add test4 2>&1};
qx{../src/task rc:dep.rc add test5 2>&1};
my $uuid = qx{../src/task rc:dep.rc _get 5.uuid};
chomp $uuid;

# [38-42] test a comma-separated list of IDs, UUIDs, and ID ranges for creation
qx{../src/task rc:dep.rc add test6 dep:1,2,3,4,$uuid 2>&1};
$output = qx{../src/task rc:dep.rc 6 info 2>&1};
like ($output, qr/test1/ms, 'Dependency appearing for task1');
like ($output, qr/test2/ms, 'Dependency appearing for task2');
like ($output, qr/test3/ms, 'Dependency appearing for task3');
like ($output, qr/test4/ms, 'Dependency appearing for task4');
like ($output, qr/test5/ms, 'Dependency appearing for task5');

# [43-47] test a comma-separated list of IDs, UUIDs, and ID ranges for deletion
qx{../src/task rc:dep.rc 6 mod dep:-1,-2,-3,-4,-$uuid 2>&1};
$output = qx{../src/task rc:dep.rc 6 info 2>&1};
unlike ($output, qr/test1/ms, 'Dependency not appearing for task1');
unlike ($output, qr/test2/ms, 'Dependency not appearing for task2');
unlike ($output, qr/test3/ms, 'Dependency not appearing for task3');
unlike ($output, qr/test4/ms, 'Dependency not appearing for task4');
unlike ($output, qr/test5/ms, 'Dependency not appearing for task5');

# TODO - test dependency.confirmation config variable
# TODO - test undo on backing out chain gap repair
# TODO - test undo on backing out choice to not perform chain gap repair
# TODO - test blocked task completion nag
# TODO - test depend.any and depend.none report filters

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data dep.rc);
exit 0;

