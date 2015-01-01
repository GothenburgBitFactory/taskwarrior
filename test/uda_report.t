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
if (open my $fh, '>', 'uda.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "uda.extra.type=string\n",
            "uda.extra.label=Extra\n",
            "report.good.columns=id,extra\n",
            "report.good.description=Test report\n",
            "report.good.filter=\n",
            "report.good.labels=ID,Extra\n",
            "report.good.sort=ID\n",
            "report.bad.columns=id,extra2\n",
            "report.bad.description=Test report2\n",
            "report.bad.filter=\n",
            "report.bad.labels=ID,Extra2\n",
            "report.bad.sort=ID\n";
  close $fh;
}

# Add a task with a defined UDA.
qx{../src/task rc:uda.rc add one extra:foo 2>&1};

# Run a report that references the UDA.
my $output = qx{../src/task rc:uda.rc good 2>&1};
like ($output, qr/foo/, 'UDA shown in report');

# Run a report that references an Orphan UDA.
$output = qx{../src/task rc:uda.rc bad 2>&1};
like ($output, qr/Unrecognized column name/, 'UDA Orphan causes error');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data uda.rc import.txt);
exit 0;

