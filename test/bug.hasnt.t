#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 16;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'hasnt.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'hasnt.rc', 'Created hasnt.rc');
}

# 1
qx{../src/task rc:hasnt.rc add foo 2>&1};

# 2
qx{../src/task rc:hasnt.rc add foo 2>&1};
qx{../src/task rc:hasnt.rc 2 annotate bar 2>&1};

# 3
qx{../src/task rc:hasnt.rc add foo 2>&1};
qx{../src/task rc:hasnt.rc 3 annotate bar 2>&1};
qx{../src/task rc:hasnt.rc 3 annotate baz 2>&1};

# 4
qx{../src/task rc:hasnt.rc add bar 2>&1};

# 5
qx{../src/task rc:hasnt.rc add bar 2>&1};
qx{../src/task rc:hasnt.rc 5 annotate foo 2>&1};

# 6
qx{../src/task rc:hasnt.rc add bar 2>&1};
qx{../src/task rc:hasnt.rc 6 annotate foo 2>&1};
qx{../src/task rc:hasnt.rc 6 annotate baz 2>&1};

#7
qx{../src/task rc:hasnt.rc add one 2>&1};
qx{../src/task rc:hasnt.rc 7 annotate two 2>&1};
qx{../src/task rc:hasnt.rc 7 annotate three 2>&1};

my $output = qx{../src/task rc:hasnt.rc ls description.has:foo 2>&1};
like   ($output, qr/\n 1/, '1 has foo -> yes');
like   ($output, qr/\n 2/, '2 has foo -> yes');
like   ($output, qr/\n 3/, '3 has foo -> yes');
unlike ($output, qr/\n 4/, '4 has foo -> no');  # 5
like   ($output, qr/\n 5/, '5 has foo -> yes');
like   ($output, qr/\n 6/, '6 has foo -> yes');
unlike ($output, qr/\n 7/, '7 has foo -> no');

$output = qx{../src/task rc:hasnt.rc ls description.hasnt:foo 2>&1};
unlike ($output, qr/\n 1/, '1 hasnt foo -> no');
unlike ($output, qr/\n 2/, '2 hasnt foo -> no');  # 10
unlike ($output, qr/\n 3/, '3 hasnt foo -> no');
like   ($output, qr/\n 4/, '4 hasnt foo -> yes');
unlike ($output, qr/\n 5/, '5 hasnt foo -> no');
unlike ($output, qr/\n 6/, '6 hasnt foo -> no');
like   ($output, qr/\n 7/, '7 hasnt foo -> yes');  # 15

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data hasnt.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'hasnt.rc', 'Cleanup');

exit 0;

