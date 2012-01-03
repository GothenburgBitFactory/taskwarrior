#! /usr/bin/perl
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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'ids.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'ids.rc', 'Created ids.rc');
}

# Test the count command.
qx{../src/task rc:ids.rc add one    +A +B};
qx{../src/task rc:ids.rc add two    +A   };
qx{../src/task rc:ids.rc add three  +A +B};
qx{../src/task rc:ids.rc add four        };
qx{../src/task rc:ids.rc add five   +A +B};

my $output = qx{../src/task rc:ids.rc ids +A};
like ($output, qr/^1-3,5$/ms, 'ids +A --> 1-3,5');

$output = qx{../src/task rc:ids.rc ids +B};
like ($output, qr/^1,3,5$/ms, 'ids +B --> 1,3,5');

$output = qx{../src/task rc:ids.rc ids +A -B};
like ($output, qr/^2$/ms, 'ids +A -B --> 2');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key ids.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'ids.rc', 'Cleanup');

exit 0;

