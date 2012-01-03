#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 45;

# Create the rc file.
if (open my $fh, '>', 'bulk.rc')
{
  print $fh "data.location=.\n",
            "bulk=3\n";
  close $fh;
  ok (-r 'bulk.rc', 'Created bulk.rc');
}

# Exercise bulk and non-bulk confirmations for 'delete' and 'modify'.
qx{../src/task rc:bulk.rc add one};
qx{../src/task rc:bulk.rc add two};
qx{../src/task rc:bulk.rc add three};
qx{../src/task rc:bulk.rc add four};
qx{../src/task rc:bulk.rc add five};
qx{../src/task rc:bulk.rc add six};
qx{../src/task rc:bulk.rc add seven};
qx{../src/task rc:bulk.rc add eight};
qx{../src/task rc:bulk.rc add nine};
qx{../src/task rc:bulk.rc add ten};
qx{../src/task rc:bulk.rc add eleven};
qx{../src/task rc:bulk.rc add twelve};
qx{../src/task rc:bulk.rc add thirteen};
qx{../src/task rc:bulk.rc add fourteen};
qx{../src/task rc:bulk.rc add fifteen};
qx{../src/task rc:bulk.rc add sixteen};
qx{../src/task rc:bulk.rc add seventeen};
qx{../src/task rc:bulk.rc add eighteen};

# The 'delete' command is used, but it could be any write command.
# Note that 'y' is passed to task despite rc.confirmation=off.  This allows
# failing tests to complete without blocking on input.

# 'yes' tests:

# Test with 1 task.  1 is a special case.
my $output = qx{echo '-- y' | ../src/task rc:bulk.rc rc.confirmation=off 1 delete};
unlike ($output, qr/\(yes\/no\)/,            'Single delete with no confirmation');
unlike ($output, qr/\(yes\/no\/all\/quit\)/, 'Single delete with no bulk confirmation');
like   ($output, qr/Deleting task 1/,        'Verified delete 1');

$output = qx{echo '-- y' | ../src/task rc:bulk.rc rc.confirmation=on 2 delete};
like   ($output, qr/\(yes\/no\)/,            'Single delete with confirmation');
unlike ($output, qr/\(yes\/no\/all\/quit\)/, 'Single delete with no bulk confirmation');
like   ($output, qr/Deleting task 2/,        'Verified delete 2');

# Test with 2 tasks.  2 is greater than 1 and less than bulk.
$output = qx{echo '-- y' | ../src/task rc:bulk.rc rc.confirmation=off 3-4 delete};
unlike ($output, qr/\(yes\/no\)/,            'Multiple delete with no confirmation');
unlike ($output, qr/\(yes\/no\/all\/quit\)/, 'Multiple delete with no bulk confirmation');
like   ($output, qr/Deleting task 3/,        'Verified delete 3');
like   ($output, qr/Deleting task 4/,        'Verified delete 4');

$output = qx{echo -e ' -- y\ny\n' | ../src/task rc:bulk.rc rc.confirmation=on 5-6 delete};
unlike ($output, qr/\(yes\/no\)/,            'Multiple delete with confirmation');
like   ($output, qr/\(yes\/no\/all\/quit\)/, 'Multiple delete with bulk confirmation');
like   ($output, qr/Deleting task 5/,        'Verified delete 5');
like   ($output, qr/Deleting task 6/,        'Verified delete 6');

# Test with 3 tasks.  3 is considered bulk.
$output = qx{echo -e ' -- y\ny\ny\n' | ../src/task rc:bulk.rc rc.confirmation=off 7-9 delete};
unlike ($output, qr/\(yes\/no\)/,            'Bulk delete with no confirmation');
like   ($output, qr/\(yes\/no\/all\/quit\)/, 'Bulk delete with no bulk confirmation');
like   ($output, qr/Deleting task 7/,        'Verified delete 7');
like   ($output, qr/Deleting task 8/,        'Verified delete 8');
like   ($output, qr/Deleting task 9/,        'Verified delete 9');

$output = qx{echo -e ' -- y\ny\ny\n' | ../src/task rc:bulk.rc rc.confirmation=on 10-12 delete};
unlike ($output, qr/\(yes\/no\)/,            'Bulk delete with confirmation');
like   ($output, qr/\(yes\/no\/all\/quit\)/, 'Bulk delete with bulk confirmation');
like   ($output, qr/Deleting task 10/,       'Verified delete 10');
like   ($output, qr/Deleting task 11/,       'Verified delete 11');
like   ($output, qr/Deleting task 12/,       'Verified delete 12');

# 'no' tests:

# Test with 1 task, denying delete.
$output = qx{echo '-- n' | ../src/task rc:bulk.rc rc.confirmation=on 13 delete};
like   ($output, qr/\(yes\/no\)/,            'Single delete with confirmation');
unlike ($output, qr/\(yes\/no\/all\/quit\)/, 'Single delete with no bulk confirmation');
unlike ($output, qr/Deleting task/,          'Verified no delete 13');

# Test with 2 tasks, denying delete.
$output = qx{echo -e ' -- n\nn\n' | ../src/task rc:bulk.rc rc.confirmation=on 13-14 delete};
unlike ($output, qr/\(yes\/no\)/,            'Multiple delete with confirmation');
like   ($output, qr/\(yes\/no\/all\/quit\)/, 'Multiple delete with no bulk confirmation');
unlike ($output, qr/Deleting task/,          'Verified no delete 13-14');

# Test with 3 tasks, denying delete.
$output = qx{echo -e ' -- n\nn\nn\n' | ../src/task rc:bulk.rc rc.confirmation=on 13-15 delete};
unlike ($output, qr/\(yes\/no\)/,            'Bulk delete with confirmation');
like   ($output, qr/\(yes\/no\/all\/quit\)/, 'Bulk delete with no bulk confirmation');
unlike ($output, qr/Deleting task/,          'Verified no delete 13-15');

# 'all' tests:
$output = qx{echo '-- all' | ../src/task rc:bulk.rc rc.confirmation=on 13-15 delete};
unlike ($output, qr/\(yes\/no\)/,            'Bulk delete with confirmation');
like   ($output, qr/\(yes\/no\/all\/quit\)/, 'Bulk delete with bulk confirmation');
like   ($output, qr/Deleting task/,          'Verified delete 13');
like   ($output, qr/Deleting task/,          'Verified delete 14');
like   ($output, qr/Deleting task/,          'Verified delete 15');

# 'quit' tests:
$output = qx{echo '-- quit' | ../src/task rc:bulk.rc rc.confirmation=on 16-18 delete};
unlike ($output, qr/\(yes\/no\)/,            'Bulk delete with no confirmation');
like   ($output, qr/\(yes\/no\/all\/quit\)/, 'Bulk delete with no bulk confirmation');
unlike ($output, qr/Deleting task/,          'Verified delete 16');
unlike ($output, qr/Deleting task/,          'Verified delete 17');
unlike ($output, qr/Deleting task/,          'Verified delete 18');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bulk.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bulk.rc', 'Cleanup');

exit 0;

