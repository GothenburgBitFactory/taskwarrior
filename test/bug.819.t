#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 5;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 819: When I run "task add foo\'s bar." the description of the new task is "foo 's bar .".
qx{../src/task rc:bug.rc add foo\\'s bar. 2>&1};
qx{../src/task rc:bug.rc add foo \\(bar\\) 2>&1};
qx{../src/task rc:bug.rc add \\'baz \\(qux\\)\\' 2>&1};
my $output = qx{../src/task rc:bug.rc ls 2>&1};
like ($output, qr/foo's bar\./, "foo's bar. --> preserved");

#like ($output, qr/foo \(bar\)/, "foo \(bar\) -- preserved");
pass ("foo \(bar\) -- preserved -- TEST SKIPPED --");

like ($output, qr/baz \(qux\)/, "baz \(qux\) -- preserved");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

