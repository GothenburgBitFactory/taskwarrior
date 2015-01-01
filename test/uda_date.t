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
if (open my $fh, '>', 'uda.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "uda.extra.type=date\n",
            "uda.extra.label=Extra\n",
            "report.uda.description=UDA Test\n",
            "report.uda.columns=id,extra,description\n",
            "report.uda.sort=extra,description\n",
            "report.uda.labels=ID,Extra,Description\n",
            "dateformat=m/d/Y\n";
  close $fh;
}

# Add tasks with and without the UDA.
qx{../src/task rc:uda.rc add with extra:tomorrow 2>&1};
qx{../src/task rc:uda.rc add without 2>&1};
my $output = qx{../src/task rc:uda.rc uda 2>&1};
like ($output, qr/1\s+[\d\/]+\s+with/, 'UDA date stored');
like ($output, qr/2\s+without/,        'UDA date blank');

# Add bad data.
$output = qx{../src/task rc:uda.rc add bad extra:unrecognized_date 2>&1};
unlike ($output, qr/Created task \d+/, 'UDA date bad data not accepted');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data uda.rc);
exit 0;

