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
if (open my $fh, '>', 'tags.rc')
{
  print $fh "data.location=.\n",
            "defaultwidth=100\n";
  close $fh;
}

# Create a data set of two tasks, with unique project names, one
# pending, one completed.
qx{../src/task rc:tags.rc add +t1 one 2>&1};
qx{../src/task rc:tags.rc add +t2 two 2>&1};
qx{../src/task rc:tags.rc 1 done 2>&1};
my $output = qx{../src/task rc:tags.rc long 2>&1};
unlike ($output, qr/t1/, 't1 done');
like ($output, qr/t2/, 't2 pending');

$output = qx{../src/task rc:tags.rc tags 2>&1};
unlike ($output, qr/t1/, 't1 done');
like ($output, qr/t2/, 't2 pending');

$output = qx{../src/task rc:tags.rc rc.list.all.tags:yes tags 2>&1};
like ($output, qr/t1/, 't1 listed');
like ($output, qr/t2/, 't2 listed');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data  tags.rc);
exit 0;

