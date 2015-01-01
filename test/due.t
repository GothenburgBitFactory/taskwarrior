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
use Test::More tests => 4;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'due.rc')
{
  print $fh "data.location=.\n",
            "due=4\n",
            "color=on\n",
            "color.due=red\n",
            "color.alternate=\n",
            "_forcecolor=on\n",
            "dateformat=m/d/Y\n";
  close $fh;
}

# Add a task that is almost due, and one that is just due.
my ($d, $m, $y) = (localtime (time + 3 * 86_400))[3..5];
my $just = sprintf ("%d/%d/%d", $m + 1, $d, $y + 1900);

($d, $m, $y) = (localtime (time + 5 * 86_400))[3..5];
my $almost = sprintf ("%d/%d/%d", $m + 1, $d, $y + 1900);

qx{../src/task rc:due.rc add one due:$just 2>&1};
qx{../src/task rc:due.rc add two due:$almost 2>&1};
my $output = qx{../src/task rc:due.rc list 2>&1};
like ($output, qr/\[31m.+$just.+\[0m/, 'one marked due');
like ($output, qr/\s+$almost\s+/, 'two not marked due');

qx{../src/task rc:due.rc add three due:today 2>&1};
$output = qx{../src/task rc:due.rc list due:today 2>&1};
like ($output, qr/three/, 'due:today works as a filter');

$output = qx{../src/task rc:due.rc list due.is:today 2>&1};
like ($output, qr/three/, 'due.is:today works as a filter');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data due.rc);
exit 0;

