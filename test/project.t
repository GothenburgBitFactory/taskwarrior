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
use Test::More tests => 11;

# Create the rc file.
if (open my $fh, '>', 'pro.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'pro.rc', 'Created pro.rc');
}

# Test the project status numbers.
my $output = qx{../src/task rc:pro.rc add one pro:foo};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 0% complete \(1 of 1 tasks remaining\)\./, 'add one');

$output = qx{../src/task rc:pro.rc add two pro:'foo'};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 0% complete \(2 of 2 tasks remaining\)\./, 'add two');

$output = qx{../src/task rc:pro.rc add three pro:'foo'};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 0% complete \(3 of 3 tasks remaining\)\./, 'add three');

$output = qx{../src/task rc:pro.rc add four pro:'foo'};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 0% complete \(4 of 4 tasks remaining\)\./, 'add four');

$output = qx{../src/task rc:pro.rc 1 done};
like ($output, qr/Project 'foo' is 25% complete \(3 of 4 tasks remaining\)\./, 'done one');

$output = qx{../src/task rc:pro.rc 2 delete};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 33% complete \(2 of 3 tasks remaining\)\./, 'delete two');

$output = qx{../src/task rc:pro.rc 3 modify pro:bar};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 50% complete \(1 of 2 tasks remaining\)\./, 'change project');
like ($output, qr/The project 'bar' has changed\.  Project 'bar' is 0% complete \(1 of 1 tasks remaining\)\./, 'change project');

# Test projects with spaces in them.
$output = qx{../src/task rc:pro.rc 3 modify pro:\\"foo bar\\"};
like ($output, qr/The project 'foo bar' has changed\./, 'project with spaces');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key pro.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'pro.rc', 'Cleanup');

exit 0;

