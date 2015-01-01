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
if (open my $fh, '>', 'recur.rc')
{
  print $fh "data.location=.\n",
            "bulk=10\n",
            "dateformat=Y-M-DTH:N:S\n";
  close $fh;
}

# Create a few recurring tasks, and test the sort order of the recur column.

if (open my $fh, '>', 'pending.data')
{
  my $until = time () - 2;
  my $entry = $until - 5;

  print $fh <<EOF;
[uuid:"00000000-0000-0000-0000-000000000000" status:"recurring" description:"foo" entry:"$entry" due:"$entry" recur:"PT2S" until:"$until"]
EOF
  close $fh;
}

my $output = qx{../src/task rc:recur.rc list 2>&1};
like ($output, qr/^\s+2/ms, 'Found 2');
like ($output, qr/^\s+3/ms, 'Found 3');
like ($output, qr/^\s+4/ms, 'Found 4');
like ($output, qr/^\s+5/ms, 'Found 5');

# There may be more than 4, but we don't care, because they must all now be
# removed.
qx{../src/task rc:recur.rc status:pending and /foo/ done 2>&1};

$output = qx{../src/task rc:recur.rc list 2>&1};
like ($output, qr/and was deleted/, 'Parent task deleted');

$output = qx{../src/task rc:recur.rc diag 2>&1};
like ($output, qr/No duplicates found/, 'No duplicate UUIDs detected');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data recur.rc);
exit 0;

