#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2014, Paul Beckingham, Federico Hernandez.
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
use Time::Local;
use Test::More tests => 17;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'before.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n",
            "dateformat=m/d/Y\n",
            "dateformat.info=m/d/Y\n";
  close $fh;
  ok (-r 'before.rc', 'Created before.rc');
}

# Create some example data directly.
if (open my $fh, '>', 'pending.data')
{
  my $timeA = timegm (00,00,12,22,11,2008);
  my $timeB = timegm (00,00,12,17,03,2009);
  print $fh <<EOF;
[description:"foo" entry:"$timeA" start:"$timeA" status:"pending" uuid:"27097693-91c2-4cbe-ba89-ddcc87e5582c"]
[description:"bar" entry:"$timeB" start:"$timeB" status:"pending" uuid:"08f72d91-964c-424b-8fd5-556434648b6b"]
EOF

  close $fh;
  ok (-r 'pending.data', 'Created pending.data');
}

# Verify data is readable and just as expected.
my $output = qx{../src/task rc:before.rc 1 info 2>&1};
like ($output, qr/Start\s+12\/22\/2008/, 'task 1 start date as expected');

$output = qx{../src/task rc:before.rc 2 info 2>&1};
like ($output, qr/Start\s+4\/17\/2009/, 'task 2 start date as expected');

$output = qx{../src/task rc:before.rc ls start.before:12/1/2008 2>&1};
unlike ($output, qr/foo/, 'no foo before 12/1/2008');
unlike ($output, qr/bar/, 'no bar before 12/1/2008');
$output = qx{../src/task rc:before.rc ls start.before:1/1/2009 2>&1};
like ($output, qr/foo/, 'foo before 1/1/2009');
unlike ($output, qr/bar/, 'no bar before 1/1/2009');
$output = qx{../src/task rc:before.rc ls start.before:5/1/2009 2>&1};
like ($output, qr/foo/, 'foo before 5/1/2009');
like ($output, qr/bar/, 'bar before 5/1/2009');
$output = qx{../src/task rc:before.rc ls start.after:12/1/2008 2>&1};
like ($output, qr/foo/, 'foo after 12/1/2008');
like ($output, qr/bar/, 'bar after 12/1/2008');
$output = qx{../src/task rc:before.rc ls start.after:1/1/2009 2>&1};
unlike ($output, qr/foo/, 'no foo after 1/1/2009');
like ($output, qr/bar/, 'bar after 1/1/2009');
$output = qx{../src/task rc:before.rc ls start.after:5/1/2009 2>&1};
unlike ($output, qr/foo/, 'no foo after 5/1/2009');
unlike ($output, qr/bar/, 'no bar after 5/1/2009');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data before.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'before.rc', 'Cleanup');

exit 0;

