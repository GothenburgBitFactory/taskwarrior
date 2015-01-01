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
use Test::More tests => 20;

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
            "dateformat=YMD\n",
            "dateformat.info=YMD\n",
            "confirmation=off\n";
  close $fh;
}

# DOM reference to other task.
qx{../src/task rc:$rc add one due:20110901 2>&1};
qx{../src/task rc:$rc add two due:1.due 2>&1};
my $output = qx{../src/task rc:$rc 2 info 2>&1};
like ($output, qr/Due\s+20110901/, "$ut: Found due date duplicated via dom");

# DOM reference to the current task.
qx{../src/task rc:$rc add three due:20110901 wait:due +tag1 +tag2 2>&1};
$output = qx{../src/task rc:$rc 3 info 2>&1};
like ($output, qr/Waiting until\s+20110901/, "$ut: Found wait date duplicated from due date");

# ID <--> UUID <--> ID round trip via DOM.
$output = qx{../src/task rc:$rc _get 1.uuid 2>&1};
like ($output, qr/^.{36}$/, "$ut: DOM id --> uuid");
my $uuid = chomp $output;
$output = qx{../src/task rc:$rc _get ${uuid}.id 2>&1};
like ($output, qr/^1$/, "$ut: DOM uuid --> id");

# Failed DOM lookup returns blank.
$output = qx{../src/task rc:$rc _get 4.description 2>&1};
like ($output, qr/^$/, "DOM 4.description --> ''");

# Test extended DOM support (2.4.0)
$output = qx{../src/task rc:$rc _get 3.tags 2>&1};
like ($output, qr/^tag1,tag2$/, "$ut: <id>.<tags>");

$output = qx{../src/task rc:$rc _get 3.tags.tag1 2>&1};
like ($output, qr/^tag1$/, "$ut: <id>.tags.tag1");

$output = qx{../src/task rc:$rc _get 3.tags.OVERDUE 2>&1};
like ($output, qr/^OVERDUE$/, "$ut: <id>.tags.<tag>");

$output = qx{../src/task rc:$rc _get 3.due.year 2>&1};
like ($output, qr/^\d{4}$/, "$ut: <id>.due.year");

$output = qx{../src/task rc:$rc _get 3.due.month 2>&1};
like ($output, qr/^\d{1,2}$/, "$ut: <id>.due.month");

$output = qx{../src/task rc:$rc _get 3.due.day 2>&1};
like ($output, qr/^\d{1,2}$/, "$ut: <id>.due.day");

$output = qx{../src/task rc:$rc _get 3.due.week 2>&1};
like ($output, qr/^\d{1,2}$/, "$ut: <id>.due.week");

$output = qx{../src/task rc:$rc _get 3.due.weekday 2>&1};
like ($output, qr/^\d{1}$/, "$ut: <id>.due.weekday");

$output = qx{../src/task rc:$rc _get 3.due.hour 2>&1};
like ($output, qr/^\d{1,2}$/, "$ut: <id>.due.hour");

$output = qx{../src/task rc:$rc _get 3.due.minute 2>&1};
like ($output, qr/^\d{1,2}$/, "$ut: <id>.due.minute");

$output = qx{../src/task rc:$rc _get 3.due.second 2>&1};
like ($output, qr/^\d{1,2}$/, "$ut: <id>.due.second");

$output = qx{../src/task rc:$rc _get 3.due.year 2>&1};
like ($output, qr/^\d{4}$/, "$ut: <id>.due.year");

qx{../src/task rc:$rc 3 annotate note 2>&1};
ok ($? == 0, "$ut: add annotation");

$output = qx{../src/task rc:$rc _get 3.annotations.1.entry 2>&1};
like ($output, qr/^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}$/, "$ut: <id>.annotations.1.entry");

$output = qx{../src/task rc:$rc _get 3.annotations.1.description 2>&1};
like ($output, qr/^note$/, "$ut: <id>.annotations.1.description");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

