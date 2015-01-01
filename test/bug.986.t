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

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "verbose=nothing\n";
  close $fh;
}

# Bug 986 - 'task info' does not format date using dateformat.report
# Rely on the assumption that the default date format is 'm/d/Y'

# Create one task (with a creation date) and one journal entry (with a
# timestamp and a date inside the entry)
qx{../src/task rc:$rc add test 2>&1};
qx{../src/task rc:$rc test start 2>&1};

# Test that dateformat.info has precedence over dateformat and that no other
# format is applied
my $output = qx{../src/task rc:$rc test info rc.dateformat:m/d/Y rc.dateformat.info:__ 2>&1};
like   ($output, qr/__/ms,                       "$ut: Date formatted according to dateformat.info");
unlike ($output, qr/[0-9]*\/[0-9]*\/20[0-9]*/ms, "$ut: No date is incorrectly formatted");

# Similar for dateformat
$output = qx{../src/task rc:$rc test info rc.dateformat:__ rc.dateformat.info: 2>&1};
like   ($output, qr/__/ms,                       "$ut: Date formatted according to dateformat");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
