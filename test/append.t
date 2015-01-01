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
use Test::More tests => 4;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'append.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
}

# Add a task, then append more description.
qx{../src/task rc:append.rc add foo 2>&1};
my $output = qx{../src/task rc:append.rc 1 append bar 2>&1};
like ($output, qr/^Appended 1 task.$/m, 'append worked');
$output = qx{../src/task rc:append.rc info 1 2>&1};
like ($output, qr/Description\s+foo\sbar\n/, 'append worked');

# Should cause an error when nothing is appended.
$output = qx{../src/task rc:append.rc 1 append 2>&1};
like ($output, qr/^Additional text must be provided.$/m, 'blank append failed');
unlike ($output, qr/^Appended 1 task.$/, 'blank append failed');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data append.rc);
exit 0;

