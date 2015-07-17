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
use Test::More tests => 33;

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
            "confirmation=off\n",
            "verbose=nothing\n";
  close $fh;
}

# Add task with tags.
my $output = qx{../src/task rc:$rc add +one This +two is a test +three 2>&1; ../src/task rc:$rc info 1 2>&1};
like ($output, qr/^Tags\s+one two three\n/m, "$ut: tags found");

# Remove tags.
$output = qx{../src/task rc:$rc 1 modify -three -two -one 2>&1; ../src/task rc:$rc info 1 2>&1};
unlike ($output, qr/^Tags/m, "$ut: -three -two -one tag removed");

# Add tags.
$output = qx{../src/task rc:$rc 1 modify +four +five +six 2>&1; ../src/task rc:$rc info 1 2>&1};
like ($output, qr/^Tags\s+four five six\n/m, "$ut: tags found");

# Remove tags.
$output = qx{../src/task rc:$rc 1 modify -four -five -six 2>&1; ../src/task rc:$rc info 1 2>&1};
unlike ($output, qr/^Tags/m, "$ut: -four -five -six tag removed");

# Add and remove tags.
$output = qx{../src/task rc:$rc 1 modify +duplicate -duplicate 2>&1; ../src/task rc:$rc info 1 2>&1};
unlike ($output, qr/^Tags/m, "$ut: +duplicate -duplicate NOP");

# Remove missing tag.
$output = qx{../src/task rc:$rc 1 modify -missing 2>&1; ../src/task rc:$rc info 1 2>&1};
unlike ($output, qr/^Tags/m, "$ut: -missing NOP");

# Virtual tag testing.
qx{../src/task rc:$rc log completed 2>&1};
qx{../src/task rc:$rc add deleted 2>&1; ../src/task rc:$rc 2 delete 2>&1};
qx{../src/task rc:$rc add minimal 2>&1};
qx{../src/task rc:$rc add maximal +tag pro:PRO pri:H due:yesterday 2>&1};
qx{../src/task rc:$rc 4 start 2>&1};
qx{../src/task rc:$rc 4 annotate note 2>&1};
qx{../src/task rc:$rc add blocked depends:1 2>&1};
qx{../src/task rc:$rc add due_eom due:eom 2>&1};
qx{../src/task rc:$rc add due_eow due:eow 2>&1};

$output = qx{../src/task rc:$rc +COMPLETED all};
like ($output, qr/completed/, "$ut: +COMPLETED");
$output = qx{../src/task rc:$rc -COMPLETED all};
unlike ($output, qr/completed/, "$ut: -COMPLETED");

$output = qx{../src/task rc:$rc +DELETED all};
like ($output, qr/deleted/, "$ut: +DELETED");
$output = qx{../src/task rc:$rc -DELETED all};
unlike ($output, qr/deleted/, "$ut: -DELETED");

$output = qx{../src/task rc:$rc +PENDING all};
like ($output, qr/minimal/, "$ut: +PENDING");
$output = qx{../src/task rc:$rc -PENDING all};
unlike ($output, qr/minimal/, "$ut: -PENDING");

$output = qx{../src/task rc:$rc +TAGGED list};
like ($output, qr/maximal/, "$ut: +TAGGED");
$output = qx{../src/task rc:$rc -TAGGED list};
unlike ($output, qr/maximal/, "$ut: -TAGGED");

$output = qx{../src/task rc:$rc +OVERDUE list};
like ($output, qr/maximal/, "$ut: +OVERDUE");
$output = qx{../src/task rc:$rc -OVERDUE list};
unlike ($output, qr/maximal/, "$ut: -OVERDUE");

$output = qx{../src/task rc:$rc +BLOCKED list};
like ($output, qr/blocked/, "$ut: +BLOCKED");
$output = qx{../src/task rc:$rc -BLOCKED list};
unlike ($output, qr/blocked/, "$ut: -BLOCKED");

$output = qx{../src/task rc:$rc +BLOCKING list};
like ($output, qr/This is a test/, "$ut: +BLOCKING");
$output = qx{../src/task rc:$rc -BLOCKING list};
unlike ($output, qr/This is a test/, "$ut: -BLOCKING");

$output = qx{../src/task rc:$rc +UNBLOCKED list};
like ($output, qr/minimal/, "$ut: +UNBLOCKED");
$output = qx{../src/task rc:$rc -UNBLOCKED list};
unlike ($output, qr/minimal/, "$ut: -UNBLOCKED");

$output = qx{../src/task rc:$rc +YEAR list};
like ($output, qr/due_eom/, "$ut: +YEAR");
$output = qx{../src/task rc:$rc -YEAR list};
unlike ($output, qr/due_eom/, "$ut: -YEAR");

$output = qx{../src/task rc:$rc +MONTH list};
like ($output, qr/due_eom/, "$ut: +MONTH");
$output = qx{../src/task rc:$rc -MONTH list};
unlike ($output, qr/due_eom/, "$ut: -MONTH");

$output = qx{../src/task rc:$rc +WEEK list};
like ($output, qr/due_eow/, "$ut: +WEEK");
$output = qx{../src/task rc:$rc -WEEK list};
unlike ($output, qr/due_eow/, "$ut: -WEEK");

$output = qx{../src/task rc:$rc +ACTIVE list};
like ($output, qr/maximal/, "$ut: +ACTIVE");
$output = qx{../src/task rc:$rc -ACTIVE list};
unlike ($output, qr/maximal/, "$ut: -ACTIVE");

$output = qx{../src/task rc:$rc +ANNOTATED list};
like ($output, qr/maximal/, "$ut: +ANNOTATED");
$output = qx{../src/task rc:$rc -ANNOTATED list};
unlike ($output, qr/maximal/, "$ut: -ANNOTATED");

qx{../src/task rc:$rc add seven tags:A,A,B,C,C,C};
$output = qx{../src/task rc:$rc /seven/ list};
like ($output, qr/ A B C /, 'Direct tags setting enforces uniqueness');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

