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
use Test::More tests => 2;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'next.rc')
{
  print $fh "data.location=.\n",
            "next=1\n";
  close $fh;
}

# Add two tasks for each of two projects, then run next.  There should be only
# one task from each project shown.
qx{../src/task rc:next.rc add project:A priority:H AH 2>&1};
qx{../src/task rc:next.rc add project:A priority:M AM 2>&1};
qx{../src/task rc:next.rc add project:B priority:H BH 2>&1};
qx{../src/task rc:next.rc add project:B Bnone 2>&1};

my $output = qx{../src/task rc:next.rc next 2>&1};
like ($output, qr/AH/, 'AH shown');
like ($output, qr/BH/, 'BH shown');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data next.rc);
exit 0;

