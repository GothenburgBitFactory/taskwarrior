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
            "confirmation=off\n";
  close $fh;
}

# Create some tasks that were started/finished in different months, then verify
# contents of the history report

my @timeArray = localtime(time);
my $now     = time ();
my $lastmonth   =  $now - ($timeArray[3] + 1) * 86_400;

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[status:"pending" description:"PLW" entry:"$lastmonth" wait:"$now"]
[status:"pending" description:"PL" entry:"$lastmonth"]
[status:"deleted" description:"DLN" entry:"$lastmonth" due:"$now" end:"$now"]
[status:"deleted" description:"DLN2" entry:"$lastmonth" end:"$now"]
[status:"deleted" description:"DNN" entry:"$lastmonth" end:"$now"]
[status:"completed" description:"CLN" entry:"$lastmonth" end:"$now"]
[status:"completed" description:"CLL" entry:"$lastmonth" end:"$lastmonth"]
[status:"completed" description:"CNN" entry:"$now" end:"$now"]
[status:"completed" description:"CNN2" entry:"$now" end:"$now"]
EOF
  close $fh;
}

my $output = qx{../src/task rc:$rc history.monthly 2>&1};
like ($output, qr/7\s+1\s+0\s+6/, 'history.monthly - last month');
like ($output, qr/2\s+3\s+3\s+-4/, 'history.monthly - this month');
like ($output, qr/4\s+2\s+1\s+1/, 'history.monthly - average');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
