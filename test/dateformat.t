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
if (open my $fh, '>', 'date1.rc')
{
  print $fh "data.location=.\n",
            "dateformat=YMD\n",
            "dateformat.info=YMD\n",
            "dateformat.report=YMD\n";
  close $fh;
}

if (open my $fh, '>', 'date2.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/y\n",
            "dateformat.info=m/d/y\n",
            "dateformat.report=m/d/y\n";
  close $fh;
}

if (open my $fh, '>', 'date3.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/y\n",
            "dateformat=m/d/y\n",
            "weekstart=Monday\n",
            "dateformat.info=A D B Y (wV)\n",
            "dateformat.report=A D B Y (wV)\n";
  close $fh;
}

qx{../src/task rc:date1.rc add foo due:20091231 2>&1};
my $output = qx{../src/task rc:date1.rc info 1 2>&1};
like ($output, qr/\b20091231\b/, 'date format YMD parsed');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

qx{../src/task rc:date2.rc add foo due:12/1/09 2>&1};
$output = qx{../src/task rc:date2.rc info 1 2>&1};
like ($output, qr/\b12\/1\/09\b/, 'date format m/d/y parsed');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

qx{../src/task rc:date3.rc add foo due:4/8/10 2>&1};
$output = qx{../src/task rc:date3.rc list 2>&1};
like ($output, qr/Thursday 08 April 2010 \(w14\)/, 'date format A D B Y (wV) parsed');
$output = qx{../src/task rc:date3.rc rc.dateformat.report:"D b Y - a" list 2>&1};
like ($output, qr/08 Apr 2010 - Thu/, 'date format D b Y - a parsed');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data date1.rc date2.rc date3.rc);
exit 0;

