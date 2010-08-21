#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham.
## All rights reserved.
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 2 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the
##
##     Free Software Foundation, Inc.,
##     51 Franklin Street, Fifth Floor,
##     Boston, MA
##     02110-1301
##     USA
##
################################################################################

use strict;
use warnings;
use Test::More tests => 32;

# Create the rc file.
if (open my $fh, '>', 'cal.rc')
{
  print $fh "data.location=.";

  close $fh;
  ok (-r 'cal.rc', 'Created cal.rc');
}

# Bug: The 'cal' command can fail when provided with challenging arguments.

# Should not fail (because they are correct):
my $output = qx{../task rc:cal.rc cal 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal');

# y due 2010 donkey 8
$output = qx{../task rc:cal.rc cal y 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y');
$output = qx{../task rc:cal.rc cal 8 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 8');
$output = qx{../task rc:cal.rc cal due 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal due');
$output = qx{../task rc:cal.rc cal 2010 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 2010');
$output = qx{../task rc:cal.rc cal donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal donkey');

# y due 2010 donkey 8
$output = qx{../task rc:cal.rc cal y due 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y due');
$output = qx{../task rc:cal.rc cal y 8 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y 8');
$output = qx{../task rc:cal.rc cal y 2010 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y 2010');
$output = qx{../task rc:cal.rc cal y donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y donkey');
$output = qx{../task rc:cal.rc cal 8 due 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 8 due');
$output = qx{../task rc:cal.rc cal 8 2010 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 8 2010');
$output = qx{../task rc:cal.rc cal 8 donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 8 donkey');
$output = qx{../task rc:cal.rc cal due 2010 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal due 2010');
$output = qx{../task rc:cal.rc cal due donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal due donkey');
$output = qx{../task rc:cal.rc cal 2010 donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 2010 donkey');

# y 8 due 2010 donkey
$output = qx{../task rc:cal.rc cal y 8 due 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y 8 due');
$output = qx{../task rc:cal.rc cal y 8 2010 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y 8 2010');
$output = qx{../task rc:cal.rc cal y 8 donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y 8 donkey');
$output = qx{../task rc:cal.rc cal y due 2010 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y due 2010');
$output = qx{../task rc:cal.rc cal y due donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y due donkey');
$output = qx{../task rc:cal.rc cal y 2010 donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal y 2010 donkey');
$output = qx{../task rc:cal.rc cal 8 due 2010 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 8 due 2010');
$output = qx{../task rc:cal.rc cal 8 due donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 8 due donkey');
$output = qx{../task rc:cal.rc cal 8 2010 donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal 8 2010 donkey');
$output = qx{../task rc:cal.rc cal due 2010 8 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal due 2010 8');
$output = qx{../task rc:cal.rc cal due 2010 donkey 2>&1};
unlike ($output, qr/(?:Assertion failed|Could note recognize|not a valid)/, 'cal due 2010 donkey');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'cal.rc';
ok (!-r 'cal.rc', 'Removed cal.rc');

exit 0;
