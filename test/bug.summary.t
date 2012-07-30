#! /usr/bin/env perl
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
use Test::More tests => 4;

# Create the rc file.
if (open my $fh, '>', 'summary.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'summary.rc', 'Created summary.rc');
}

# Add three tasks.  Do 1, delete 1, leave 1 pending.  Summary should depict a
# 50% completion.
qx{../src/task rc:summary.rc add project:A one 2>&1};
qx{../src/task rc:summary.rc add project:A two 2>&1};
qx{../src/task rc:summary.rc add project:A three 2>&1};
qx{../src/task rc:summary.rc 1 do 2>&1};
qx{../src/task rc:summary.rc 2 delete 2>&1};
my $output = qx{../src/task rc:summary.rc summary 2>&1};
like ($output, qr/A\s+1\s+(?:-|\d\ssecs?)\s+50%/, 'summary correctly shows 50% before report');

qx{../src/task rc:summary.rc list 2>&1};
$output = qx{../src/task rc:summary.rc summary 2>&1};
like ($output, qr/A\s+1\s+(?:-|\d\ssecs?)\s+50%/, 'summary correctly shows 50% after report');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key summary.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'summary.rc', 'Cleanup');

exit 0;

