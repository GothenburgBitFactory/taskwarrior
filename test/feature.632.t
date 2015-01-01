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
use Test::More tests => 10;
use File::Path;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc files.
if (open my $fh, '>', 'rc1')
{
  print $fh "data.location=./data1\n";
  close $fh;
}

if (open my $fh, '>', 'rc2')
{
  print $fh "data.location=./data2\n";
  close $fh;
  ok (-r 'rc2', 'Created rc2');
}

# Feature #632: task environment variables: TASKRC and TASKDATA
qx{../src/task rc:rc1 add one 2>&1};
qx{../src/task rc:rc2 add two 2>&1};

# All in agreement: 1
my $output = qx{../src/task rc:rc1 list 2>&1};
like ($output, qr/one/, 'rc1');

$output = qx{TASKDATA=./data1 ../src/task rc:rc1 list 2>&1};
like ($output, qr/one/, 'TASKDATA, rc1');

$output = qx{TASKRC=./rc1 ../src/task list 2>&1};
like ($output, qr/one/, 'TASKRC');

$output = qx{TASKDATA=./data1 TASKRC=./rc1 ../src/task list 2>&1};
like ($output, qr/one/, 'TASKDATA, TASKRC, rc1');

# All in agreement: 2
$output = qx{../src/task rc:rc2 list 2>&1};
like ($output, qr/two/, 'rc2');

$output = qx{TASKDATA=./data2 ../src/task rc:rc2 list 2>&1};
like ($output, qr/two/, 'TASKDATA, rc2');

$output = qx{TASKRC=./rc2 ../src/task list 2>&1};
like ($output, qr/two/, 'TASKRC');

$output = qx{TASKDATA=./data2 TASKRC=./rc2 ../src/task list 2>&1};
like ($output, qr/two/, 'TASKDATA, TASKRC, rc2');

# rc: overrides TASKRC, TASKDATA
$output = qx{TASKDATA=./data1 TASKRC=./rc1 ../src/task rc:rc2 list 2>&1};
like ($output, qr/one/, 'overrides TASKDATA, TASKRC override rc:');

rmtree ('./data1', 0 , 1);
rmtree ('./data2', 0 , 1);

unlink qw(rc1 rc2);
exit 0

