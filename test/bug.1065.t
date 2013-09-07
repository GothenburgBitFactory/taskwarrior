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
use Test::More tests => 14;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n";
  print $fh "alias.xyzzyx=status:waiting\n";
  print $fh "imnotrecognized=kai\n";

  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 1065 - CmdShow should not display the differ message if no non-default in matched elements.
my $output = qx{../src/task rc:bug.rc show alias 2>&1};
ok ($? == 0, 'Exit status check');
like ($output, qr/Some of your .taskrc variables differ from the default values./, 'Message is shown when non-default matches in pattern');

$output = qx{../src/task rc:bug.rc show 2>&1};
ok ($? == 0, 'Exit status check');
like ($output, qr/Some of your .taskrc variables differ from the default values./, 'Message is shown when non-default matches in all');

$output = qx{../src/task rc:bug.rc show report.overdue 2>&1};
ok ($? == 0, 'Exit status check');
unlike ($output, qr/Some of your .taskrc variables differ/, 'Message is not shown when no non-default matches in pattern');

# Bug 1065 - CmdShow should not display the unrecognized message if no non-default in matched elements.
$output = qx{../src/task rc:bug.rc show notrecog 2>&1};
ok ($? == 0, 'Exit status check');
like ($output, qr/Your .taskrc file contains these unrecognized variables:/, 'Message is shown when unrecognized matches in pattern');

$output = qx{../src/task rc:bug.rc show 2>&1};
ok ($? == 0, 'Exit status check');
like ($output, qr/Your .taskrc file contains these unrecognized variables:/, 'Message is shown when unrecognized matches in all');

$output = qx{../src/task rc:bug.rc show report.overdue 2>&1};
ok ($? == 0, 'Exit status check');
unlike ($output, qr/unrecognized variables/, 'Message is not shown when no non-default matches in pattern');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;
