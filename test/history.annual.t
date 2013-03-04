#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

# Create the rc file.
if (open my $fh, '>', 'time.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'time.rc', 'Created time.rc');
}

# Create some tasks that were started/finished in different months, then verify
# contents of the history report

my @timeArray = localtime(time);
my $now     = time ();
my $lastyear   =  $now - ($timeArray[7]+1) * 86_400;

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[status:"pending" description:"PLW" entry:"$lastyear" wait:"$now"]
[status:"pending" description:"PL" entry:"$lastyear"]
[status:"deleted" description:"DLN" entry:"$lastyear" due:"$now" end:"$now"]
[status:"deleted" description:"DLN2" entry:"$lastyear" end:"$now"]
[status:"deleted" description:"DNN" entry:"$lastyear" end:"$now"]
[status:"completed" description:"CLN" entry:"$lastyear" end:"$now"]
[status:"completed" description:"CLL" entry:"$lastyear" end:"$lastyear"]
[status:"completed" description:"CNN" entry:"$now" end:"$now"]
[status:"completed" description:"CNN2" entry:"$now" end:"$now"]
EOF
  close $fh;
  ok (-r 'pending.data', 'Created pending.data');
}

my $output = qx{../src/task rc:time.rc history.annual 2>&1};
like ($output, qr/7\s+1\s+0\s+6/, 'history.annual - last year');
like ($output, qr/2\s+3\s+3\s+-4/, 'history.annual - this year');
like ($output, qr/4\s+2\s+1\s+1/, 'history.annual - average');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key time.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'time.rc', 'Cleanup');

exit 0;
