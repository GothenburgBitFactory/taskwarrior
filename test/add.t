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
use Test::More tests => 9;

# Create the rc file.
if (open my $fh, '>', 'add.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'add.rc', 'Created add.rc');
}

# Test the add command.
qx{../src/task rc:add.rc add This is a test 2>&1};
my $output = qx{../src/task rc:add.rc info 1 2>&1};
like ($output, qr/ID\s+1\n/, 'add ID');
like ($output, qr/Description\s+This is a test\n/, 'add ID');
like ($output, qr/Status\s+Pending\n/, 'add Pending');
like ($output, qr/UUID\s+[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}\n/, 'add UUID');

# Test the /// modifier.
qx{../src/task rc:add.rc 1 modify /test/TEST/ 2>&1};
qx{../src/task rc:add.rc 1 modify "/is //" 2>&1};
$output = qx{../src/task rc:add.rc info 1 2>&1};
like ($output, qr/ID\s+1\n/, 'add ID');
like ($output, qr/Status\s+Pending\n/, 'add Pending');
like ($output, qr/Description\s+This a TEST\n/, 'add Description');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key add.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'add.rc', 'Cleanup');

exit 0;

