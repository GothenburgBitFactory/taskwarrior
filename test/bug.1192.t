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
use Test::More tests => 4;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  mkdir("localcopy", 0755);
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 1192 - Push fails with POSIX shell that does not do brace expansion.

# Test push.
qx{../src/task rc:bug.rc add foo 2>&1};
# completed.data doesn't get created until required.
qx{touch completed.data 2>&1};
my $output = qx{../src/task rc:bug.rc push sh+cp://localcopy/ 2>&1};
#unlike ($output, qr/_user/ms, '_user in there');
unlike ($output, qr/No such file or directory/ms, 'Local push to sh+cp://path/');

# Test pull.
$output = qx{../src/task rc:bug.rc pull sh+cp://localcopy/ 2>&1};
unlike ($output, qr/No such file or directory/ms, 'Local pull from sh+cp://path/');

## Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.rc);
qx(rm -rf localcopy/);
ok (! -d 'localcopy'      &&
    ! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;
