#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "dateformat=SNHDMY\n",
            "report.foo.columns=entry,start,end,description\n",
            "report.foo.dateformat=SNHDMY\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug #438: Reports sorting by end, start, and entry are ordered incorrectly, if
#           time is included.

# Ensure the two tasks have a 1 second delta in entry.
qx{../src/task rc:bug.rc add older};
diag ("1 second delay");
sleep 1;
qx{../src/task rc:bug.rc add newer};

my $output = qx{../src/task rc:bug.rc rc.report.foo.sort:entry+ foo};
like ($output, qr/older.+newer/ms, 'sort:entry+ -> older newer');

$output = qx{../src/task rc:bug.rc rc.report.foo.sort:entry- foo};
like ($output, qr/newer.+older/ms, 'sort:entry- -> newer older');

# Ensure the two tasks have a 1 second delta in start.
qx{../src/task rc:bug.rc 1 start};
diag ("1 second delay");
sleep 1;
qx{../src/task rc:bug.rc 2 start};

$output = qx{../src/task rc:bug.rc rc.report.foo.sort:start+ foo};
like ($output, qr/older.+newer/ms, 'sort:start+ -> older newer');

$output = qx{../src/task rc:bug.rc rc.report.foo.sort:start- foo};
like ($output, qr/newer.+older/ms, 'sort:start- -> newer older');

# Ensure the two tasks have a 1 second delta in end.
qx{../src/task rc:bug.rc 1 done};
diag ("1 second delay");
sleep 1;
qx{../src/task rc:bug.rc 2 done};

$output = qx{../src/task rc:bug.rc rc.report.foo.sort:end+ foo};
like ($output, qr/older.+newer/ms, 'sort:end+ -> older newer');

$output = qx{../src/task rc:bug.rc rc.report.foo.sort:end- foo};
like ($output, qr/newer.+older/ms, 'sort:end- -> newer older');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

