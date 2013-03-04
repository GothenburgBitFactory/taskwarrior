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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'args.rc', 'Created args.rc');
}

# Test 'done' with en-passant changes.
qx{../src/task rc:args.rc add one 2>&1};
qx{../src/task rc:args.rc add two 2>&1};
qx{../src/task rc:args.rc add three 2>&1};
qx{../src/task rc:args.rc add four 2>&1};
qx{../src/task rc:args.rc add five 2>&1};

qx{../src/task rc:args.rc 1 done oneanno 2>&1};
my $output = qx{../src/task rc:args.rc 1 info 2>&1};
like ($output, qr/oneanno/, 'done enpassant anno');

qx{../src/task rc:args.rc 2 done /two/TWO/ 2>&1};
$output = qx{../src/task rc:args.rc 2 info 2>&1};
like ($output, qr/Description\s+TWO/, 'done enpassant subst');

qx{../src/task rc:args.rc 3 done +threetag 2>&1};
$output = qx{../src/task rc:args.rc 3 info 2>&1};
like ($output, qr/Tags\s+threetag/, 'done enpassant tag');

qx{../src/task rc:args.rc 4 done pri:H 2>&1};
$output = qx{../src/task rc:args.rc 4 info 2>&1};
like ($output, qr/Priority\s+H/, 'done enpassant priority');

qx{../src/task rc:args.rc 5 done pro:A 2>&1};
$output = qx{../src/task rc:args.rc 5 info 2>&1};
like ($output, qr/Project\s+A/, 'done enpassant project');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key args.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'args.rc', 'Cleanup');

exit 0;

