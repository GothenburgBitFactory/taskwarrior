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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'args.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'args.rc', 'Created args.rc');
}

# Test 'delete' with en-passant changes.
qx{../src/task rc:args.rc add one};
qx{../src/task rc:args.rc add two};
qx{../src/task rc:args.rc add three};
qx{../src/task rc:args.rc add four};
qx{../src/task rc:args.rc add five};

qx{../src/task rc:args.rc 1 delete oneanno};
my $output = qx{../src/task rc:args.rc 1 info};
like ($output, qr/oneanno/, 'delete enpassant anno');

qx{../src/task rc:args.rc 2 delete /two/TWO/};
$output = qx{../src/task rc:args.rc 2 info};
like ($output, qr/Description\s+TWO/, 'delete enpassant subst');

qx{../src/task rc:args.rc 3 delete +threetag};
$output = qx{../src/task rc:args.rc 3 info};
like ($output, qr/Tags\s+threetag/, 'delete enpassant tag');

qx{../src/task rc:args.rc 4 delete pri:H};
$output = qx{../src/task rc:args.rc 4 info};
like ($output, qr/Priority\s+H/, 'delete enpassant priority');

qx{../src/task rc:args.rc 5 delete pro:A};
$output = qx{../src/task rc:args.rc 5 info};
like ($output, qr/Project\s+A/, 'delete enpassant project');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key args.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'args.rc', 'Cleanup');

exit 0;

