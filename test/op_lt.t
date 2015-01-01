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
use Test::More tests => 24;

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
            "confirmation=no\n";
  close $fh;
}

# Setup: Add a task
qx{../src/task rc:$rc add one   due:yesterday priority:H 2>&1};
qx{../src/task rc:$rc add two   due:tomorrow  priority:M 2>&1};
qx{../src/task rc:$rc add three               priority:L 2>&1};
qx{../src/task rc:$rc add four                           2>&1};

# Test the '<' operator.
my $output = qx{../src/task rc:$rc ls due.before:today 2>&1};
like   ($output, qr/one/,   "$ut: ls due.before:today --> one");
unlike ($output, qr/two/,   "$ut: ls due.before:today --> !two");
unlike ($output, qr/three/, "$ut: ls due.before:today --> !three");
unlike ($output, qr/four/,  "$ut: ls due.before:today --> !four");

$output = qx{../src/task rc:$rc ls 'due < today' 2>&1};
like   ($output, qr/one/,   "$ut: ls due < today --> one");
unlike ($output, qr/two/,   "$ut: ls due < today --> !two");
unlike ($output, qr/three/, "$ut: ls due < today --> !three");
unlike ($output, qr/four/,  "$ut: ls due < today --> !four");

$output = qx{../src/task rc:$rc ls priority.below:H 2>&1};
unlike ($output, qr/one/,   "$ut: ls priority.below:H --> !one");
like   ($output, qr/two/,   "$ut: ls priority.below:H --> two");
like   ($output, qr/three/, "$ut: ls priority.below:H --> three");
like   ($output, qr/four/,  "$ut: ls priority.below:H --> four");

$output = qx{../src/task rc:$rc ls 'priority < H' 2>&1};
unlike ($output, qr/one/,   "$ut: ls priority < H --> !one");
like   ($output, qr/two/,   "$ut: ls priority < H --> two");
like   ($output, qr/three/, "$ut: ls priority < H --> three");
like   ($output, qr/four/,  "$ut: ls priority < H --> four");

$output = qx{../src/task rc:$rc ls 'description < t' 2>&1};
like   ($output, qr/one/,   "$ut: ls description < t --> one");
unlike ($output, qr/two/,   "$ut: ls description < t --> !two");
unlike ($output, qr/three/, "$ut: ls description < t --> !three");
like   ($output, qr/four/,  "$ut: ls description < t --> four");

$output = qx{../src/task rc:$rc 'urgency < 10.0' ls 2>&1};
unlike ($output, qr/one/,   "$ut: ls description < 10 --> !one");
unlike ($output, qr/two/,   "$ut: ls description < 10 --> !two");
like   ($output, qr/three/, "$ut: ls description < 10 --> three");
like   ($output, qr/four/,  "$ut: ls description < 10 --> four");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

