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
use Test::More tests => 5;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
}

# Test 'stop' with en-passant changes.
qx{../src/task rc:args.rc add one 2>&1};
qx{../src/task rc:args.rc add two 2>&1};
qx{../src/task rc:args.rc add three 2>&1};
qx{../src/task rc:args.rc add four 2>&1};
qx{../src/task rc:args.rc add five 2>&1};

qx{../src/task rc:args.rc 1 start 2>&1};
qx{../src/task rc:args.rc 2 start 2>&1};
qx{../src/task rc:args.rc 3 start 2>&1};
qx{../src/task rc:args.rc 4 start 2>&1};
qx{../src/task rc:args.rc 5 start 2>&1};

qx{../src/task rc:args.rc 1 stop oneanno 2>&1};
my $output = qx{../src/task rc:args.rc 1 info 2>&1};
like ($output, qr/oneanno/, 'stop enpassant anno');

qx{../src/task rc:args.rc 2 stop /two/TWO/ 2>&1};
$output = qx{../src/task rc:args.rc 2 info 2>&1};
like ($output, qr/Description\s+TWO/, 'stop enpassant subst');

qx{../src/task rc:args.rc 3 stop +threetag 2>&1};
$output = qx{../src/task rc:args.rc 3 info 2>&1};
like ($output, qr/Tags\s+threetag/, 'stop enpassant tag');

qx{../src/task rc:args.rc 4 stop pri:H 2>&1};
$output = qx{../src/task rc:args.rc 4 info 2>&1};
like ($output, qr/Priority\s+H/, 'stop enpassant priority');

qx{../src/task rc:args.rc 5 stop pro:A 2>&1};
$output = qx{../src/task rc:args.rc 5 info 2>&1};
like ($output, qr/Project\s+A/, 'stop enpassant project');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data args.rc);
exit 0;

