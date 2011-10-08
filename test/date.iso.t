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
use Test::More tests => 4;

# Create the rc file.
if (open my $fh, '>', 'iso.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/Y\n";
  close $fh;
  ok (-r 'iso.rc', 'Created iso.rc');
}

# Test use of ISO date format, despite rc.dateformat.
qx{../src/task rc:iso.rc add one due:20110901T120000Z};
my $output = qx{../src/task rc:iso.rc 1 info};
like ($output, qr/Due\s+9\/1\/2011/, 'ISO format recognized.');

# Test use of epoch date format, despite rc.dateformat.
qx{../src/task rc:iso.rc add one due:1234524690};
$output = qx{../src/task rc:iso.rc 2 info};
like ($output, qr/Due\s+2\/13\/2009/, 'Epoch format recognized.');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key iso.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'iso.rc', 'Cleanup');

exit 0;

