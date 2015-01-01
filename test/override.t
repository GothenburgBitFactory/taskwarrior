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
use Test::More tests => 1;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'or.rc')
{
  print $fh "data.location=.\n",
            "annotations=none\n",
            "report.zzz.columns=id,due,description\n",
            "report.zzz.labels=ID,Due,Description\n",
            "report.zzz.sort=due+\n",
            "report.zzz.filter=status:pending rc.annotations:full\n";
  close $fh;
}

# The zzz report is defined with an override in the filter that contradicts
# the value in the rc.  The filter override should prevail.
qx{../src/task rc:or.rc add ONE 2>&1};
qx{../src/task rc:or.rc 1 annotate TWO 2>&1};
my $output = qx{../src/task rc:or.rc zzz 2>&1};
like ($output, qr/ONE.+TWO/ms, 'filter override > rc setting');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data or.rc);
exit 0;

