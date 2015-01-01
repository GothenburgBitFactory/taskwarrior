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
use Test::More tests => 3;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'recur.rc')
{
  print $fh "data.location=.\n",
            "report.asc.columns=id,recur,description\n",
            "report.asc.sort=recur+\n",
            "report.desc.columns=id,recur,description\n",
            "report.desc.sort=recur-\n";
  close $fh;
}

# Create a few recurring tasks, and test the sort order of the recur column.
qx{../src/task rc:recur.rc add due:tomorrow recur:daily  first 2>&1};
qx{../src/task rc:recur.rc add due:tomorrow recur:weekly second 2>&1};
qx{../src/task rc:recur.rc add due:tomorrow recur:3d     third 2>&1};

my $output = qx{../src/task rc:recur.rc asc 2>&1};
like ($output, qr/first .* third .* second/msx, 'daily 3d weekly');

$output = qx{../src/task rc:recur.rc desc 2>&1};
like ($output, qr/second .* third .* first/msx, 'weekly 3d daily');

$output = qx{../src/task rc:recur.rc diag 2>&1};
like ($output, qr/No duplicates found/, 'No duplicate UUIDs detected');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data recur.rc);
exit 0;

