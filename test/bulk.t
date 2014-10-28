#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 42;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "bulk=3\n";
  close $fh;
}

# Exercise bulk and non-bulk confirmations for 'delete' and 'modify'.
qx{../src/task rc:$rc add one 2>&1};
qx{../src/task rc:$rc add two 2>&1};
qx{../src/task rc:$rc add three 2>&1};
qx{../src/task rc:$rc add four 2>&1};
qx{../src/task rc:$rc add five 2>&1};
qx{../src/task rc:$rc add six 2>&1};
qx{../src/task rc:$rc add seven 2>&1};
qx{../src/task rc:$rc add eight 2>&1};
qx{../src/task rc:$rc add nine 2>&1};
qx{../src/task rc:$rc add ten 2>&1};
qx{../src/task rc:$rc add eleven 2>&1};
qx{../src/task rc:$rc add twelve 2>&1};
qx{../src/task rc:$rc add thirteen 2>&1};
qx{../src/task rc:$rc add fourteen 2>&1};
qx{../src/task rc:$rc add fifteen 2>&1};
qx{../src/task rc:$rc add sixteen 2>&1};
qx{../src/task rc:$rc add seventeen 2>&1};
qx{../src/task rc:$rc add eighteen 2>&1};

# The 'delete' command is used, but it could be any write command.
# Note that 'y' is passed to task despite rc.confirmation=off.  This allows
# failing tests to complete without blocking on input.

# 'yes' tests:

# Test with 1 task.  1 is a special case.
my $output = qx{echo 'y' | ../src/task rc:$rc rc.confirmation=off 1 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Single delete with no confirmation");
unlike ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Single delete with no bulk confirmation");
like   ($output, qr/Deleting task 1/,        "$ut: Verified delete 1");

$output = qx{echo 'y' | ../src/task rc:$rc rc.confirmation=on 2 delete 2>&1};
like   ($output, qr/\(yes\/no\)/,            "$ut: Single delete with confirmation");
unlike ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Single delete with no bulk confirmation");
like   ($output, qr/Deleting task 2/,        "$ut: Verified delete 2");

# Test with 2 tasks.  2 is greater than 1 and less than bulk.
$output = qx{echo 'y' | ../src/task rc:$rc rc.confirmation=off 3-4 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Multiple delete with no confirmation");
unlike ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Multiple delete with no bulk confirmation");
like   ($output, qr/Deleting task 3/,        "$ut: Verified delete 3");
like   ($output, qr/Deleting task 4/,        "$ut: Verified delete 4");

$output = qx{printf 'y\ny\n' | ../src/task rc:$rc rc.confirmation=on 5-6 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Multiple delete with confirmation");
like   ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Multiple delete with bulk confirmation");
like   ($output, qr/Deleting task 5/,        "$ut: Verified delete 5");
like   ($output, qr/Deleting task 6/,        "$ut: Verified delete 6");

# Test with 3 tasks.  3 is considered bulk.
$output = qx{printf 'y\ny\ny\n' | ../src/task rc:$rc rc.confirmation=off 7-9 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Bulk delete with no confirmation");
like   ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Bulk delete with no bulk confirmation");
like   ($output, qr/Deleting task 7/,        "$ut: Verified delete 7");
like   ($output, qr/Deleting task 8/,        "$ut: Verified delete 8");
like   ($output, qr/Deleting task 9/,        "$ut: Verified delete 9");

$output = qx{printf 'y\ny\ny\n' | ../src/task rc:$rc rc.confirmation=on 10-12 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Bulk delete with confirmation");
like   ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Bulk delete with bulk confirmation");
like   ($output, qr/Deleting task 10/,       "$ut: Verified delete 10");
like   ($output, qr/Deleting task 11/,       "$ut: Verified delete 11");
like   ($output, qr/Deleting task 12/,       "$ut: Verified delete 12");

# 'no' tests:

# Test with 1 task, denying delete.
$output = qx{echo 'n' | ../src/task rc:$rc rc.confirmation=on 13 delete 2>&1};
like   ($output, qr/\(yes\/no\)/,            "$ut: Single delete with confirmation");
unlike ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Single delete with no bulk confirmation");
unlike ($output, qr/Deleting task/,          "$ut: Verified no delete 13");

# Test with 2 tasks, denying delete.
$output = qx{printf 'n\nn\n' | ../src/task rc:$rc rc.confirmation=on 13-14 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Multiple delete with confirmation");
like   ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Multiple delete with no bulk confirmation");
unlike ($output, qr/Deleting task/,          "$ut: Verified no delete 13-14");

# Test with 3 tasks, denying delete.
$output = qx{printf 'n\nn\nn\n' | ../src/task rc:$rc rc.confirmation=on 13-15 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Bulk delete with confirmation");
like   ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Bulk delete with no bulk confirmation");
unlike ($output, qr/Deleting task/,          "$ut: Verified no delete 13-15");

# 'all' tests:
$output = qx{echo 'all' | ../src/task rc:$rc rc.confirmation=on 13-15 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Bulk delete with confirmation");
like   ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Bulk delete with bulk confirmation");
like   ($output, qr/Deleting task 13/,       "$ut: Verified delete 13");
like   ($output, qr/Deleting task 14/,       "$ut: Verified delete 14");
like   ($output, qr/Deleting task 15/,       "$ut: Verified delete 15");

# 'quit' tests:
$output = qx{echo 'quit' | ../src/task rc:$rc rc.confirmation=on 16-18 delete 2>&1};
unlike ($output, qr/\(yes\/no\)/,            "$ut: Bulk delete with no confirmation");
like   ($output, qr/\(yes\/no\/all\/quit\)/, "$ut: Bulk delete with no bulk confirmation");
like   ($output, qr/Deleted 0 tasks./,       "$ut: No task deleted");
unlike ($output, qr/delete task 17/,         "$ut: No question asked for subsequent tasks");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

