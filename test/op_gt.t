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

# Create the rc file.
if (open my $fh, '>', 'op.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
}

# Setup: Add a task
qx{../src/task rc:op.rc add one   due:yesterday priority:H 2>&1};
qx{../src/task rc:op.rc add two   due:tomorrow  priority:M 2>&1};
qx{../src/task rc:op.rc add three               priority:L 2>&1};
qx{../src/task rc:op.rc add four                           2>&1};

# Test the '>' operator.
my $output = qx{../src/task rc:op.rc ls due.after:today 2>&1};
unlike ($output, qr/one/,   'ls due.after:today --> !one');
like   ($output, qr/two/,   'ls due.after:today --> two');
unlike ($output, qr/three/, 'ls due.after:today --> !three');
unlike ($output, qr/four/,  'ls due.after:today --> !four');

$output = qx{../src/task rc:op.rc ls 'due > today' 2>&1};
unlike ($output, qr/one/,   'ls due > today --> !one');
like   ($output, qr/two/,   'ls due > today --> two');
unlike ($output, qr/three/, 'ls due > today --> !three');
unlike ($output, qr/four/,  'ls due > today --> !four');

$output = qx{../src/task rc:op.rc ls priority.above:L 2>&1};
like   ($output, qr/one/,   'ls priority.above:L --> one');
like   ($output, qr/two/,   'ls priority.above:L --> two');
unlike ($output, qr/three/, 'ls priority.above:L --> !three');
unlike ($output, qr/four/,  'ls priority.above:L --> !four');

$output = qx{../src/task rc:op.rc ls 'priority > L' 2>&1};
like   ($output, qr/one/,   'ls priority > L --> one');
like   ($output, qr/two/,   'ls priority > L --> two');
unlike ($output, qr/three/, 'ls priority > L --> !three');
unlike ($output, qr/four/,  'ls priority > L --> !four');

$output = qx{../src/task rc:op.rc ls 'description > t' 2>&1};
unlike ($output, qr/one/,   'ls description > t --> !one');
like   ($output, qr/two/,   'ls description > t --> two');
like   ($output, qr/three/, 'ls description > t --> three');
unlike ($output, qr/four/,  'ls description > t --> !four');

$output = qx{../src/task rc:op.rc 'urgency > 10.0' ls 2>&1};
like   ($output, qr/one/,   'ls urgency > 10 --> one');
like   ($output, qr/two/,   'ls urgency > 10 --> two');
unlike ($output, qr/three/, 'ls urgency > 10 --> !three');
unlike ($output, qr/four/,  'ls urgency > 10 --> !four');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data op.rc);
exit 0;

