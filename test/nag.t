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
use Test::More tests => 6;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'nag.rc')
{
  print $fh "data.location=.\n",
            "nag=NAG\n";
  close $fh;
}

my $setup = "../src/task rc:nag.rc add due:yesterday one 2>&1;"
          . "../src/task rc:nag.rc add due:tomorrow two 2>&1;"
          . "../src/task rc:nag.rc add priority:H three 2>&1;"
          . "../src/task rc:nag.rc add priority:M four 2>&1;"
          . "../src/task rc:nag.rc add priority:L five 2>&1;"
          . "../src/task rc:nag.rc add six 2>&1;";
qx{$setup};

like   (qx{../src/task rc:nag.rc 6 done 2>&1 >/dev/null}, qr/NAG/, 'done pri: -> nag');
like   (qx{../src/task rc:nag.rc 5 done 2>&1 >/dev/null}, qr/NAG/, 'done pri:L -> nag');
like   (qx{../src/task rc:nag.rc 4 done 2>&1 >/dev/null}, qr/NAG/, 'done pri:M-> nag');
like   (qx{../src/task rc:nag.rc 3 done 2>&1 >/dev/null}, qr/NAG/, 'done pri:H-> nag');
like   (qx{../src/task rc:nag.rc 2 done 2>&1 >/dev/null}, qr/NAG/, 'done due:tomorrow -> nag');
my $output = qx{../src/task rc:nag.rc 1 done 2>&1 >/dev/null};
unlike ($output, qr/NAG/, 'done due:yesterday -> no nag');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data nag.rc);
exit 0;

