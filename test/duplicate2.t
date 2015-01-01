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
use Test::More tests => 12;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'dup.rc')
{
  print $fh "data.location=.\n";
  close $fh;
}

# Add a recurring task.  Duplicate both parent and child.
qx{../src/task rc:dup.rc add R due:tomorrow recur:weekly 2>&1};
qx{../src/task rc:dup.rc list 2>&1}; # To force handleRecurrence.
my $output = qx{../src/task rc:dup.rc 1 info 2>&1};
like ($output, qr/Status\s+Recurring/, 'Found parent');
$output = qx{../src/task rc:dup.rc 2 info 2>&1};
like ($output, qr/Status\s+Pending/, 'Found child');

$output = qx{../src/task rc:dup.rc 1 duplicate 2>&1};
like ($output, qr/The duplicated task is too/, 'Duplicated parent is also a parent');

$output = qx{../src/task rc:dup.rc 2 duplicate 2>&1};
like ($output, qr/The duplicated task is not/, 'Duplicated child is also a plain task');

qx{../src/task rc:dup.rc list 2>&1}; # To force handleRecurrence.
$output = qx{../src/task rc:dup.rc 1 info 2>&1};
like ($output, qr/Status\s+Recurring/,         'Found original parent task');

$output = qx{../src/task rc:dup.rc 2 info 2>&1};
like ($output, qr/Status\s+Pending/,           'Found original child task - pending');
like ($output, qr/Parent/,                     'Found original child task - with parent');

$output = qx{../src/task rc:dup.rc 3 info 2>&1};
like ($output, qr/Status\s+Recurring/,         'Found duplicated parent task');

$output = qx{../src/task rc:dup.rc 4 info 2>&1};
like ($output, qr/Status\s+Pending/,           'Found duplicated plain task');
unlike ($output, qr/Parent/,                   'Found duplicated child task - no parent');

$output = qx{../src/task rc:dup.rc 5 info 2>&1};
like ($output, qr/Status\s+Pending/,           'Found duplicated child task');
like ($output, qr/Parent/,                     'Found duplicated child task - with parent');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data dup.rc);
exit 0;

