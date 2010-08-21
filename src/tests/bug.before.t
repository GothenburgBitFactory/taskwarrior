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
use Test::More tests => 20;

# Create the rc file.
if (open my $fh, '>', 'before.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n",
            "dateformat=m/d/Y\n";
  close $fh;
  ok (-r 'before.rc', 'Created before.rc');
}

# Create some exampel data directly.
if (open my $fh, '>', 'pending.data')
{
  # 1229947200 = 12/22/2008
  # 1240000000 = 4/17/2009
  print $fh <<EOF;
[description:"foo" entry:"1229947200" start:"1229947200" status:"pending" uuid:"27097693-91c2-4cbe-ba89-ddcc87e5582c"]
[description:"bar" entry:"1240000000" start:"1240000000" status:"pending" uuid:"08f72d91-964c-424b-8fd5-556434648b6b"]
EOF

  close $fh;
  ok (-r 'pending.data', 'Created pending.data');
}

# Verify data is readable and just as expected.
my $output = qx{../task rc:before.rc 1 info};
like ($output, qr/Start\s+12\/22\/2008/, 'task 1 start date as expected');

$output = qx{../task rc:before.rc 2 info};
like ($output, qr/Start\s+4\/17\/2009/, 'task 2 start date as expected');

$output = qx{../task rc:before.rc ls start.before:12/1/2008};
unlike ($output, qr/foo/, 'no foo before 12/1/2008');
unlike ($output, qr/bar/, 'no bar before 12/1/2008');
$output = qx{../task rc:before.rc ls start.before:1/1/2009};
like ($output, qr/foo/, 'foo before 1/1/2009');
unlike ($output, qr/bar/, 'no bar before 1/1/2009');
$output = qx{../task rc:before.rc ls start.before:5/1/2009};
like ($output, qr/foo/, 'foo before 5/1/2009');
like ($output, qr/bar/, 'bar before 5/1/2009');
$output = qx{../task rc:before.rc ls start.after:12/1/2008};
like ($output, qr/foo/, 'foo after 12/1/2008');
like ($output, qr/bar/, 'bar after 12/1/2008');
$output = qx{../task rc:before.rc ls start.after:1/1/2009};
unlike ($output, qr/foo/, 'no foo after 1/1/2009');
like ($output, qr/bar/, 'bar after 1/1/2009');
$output = qx{../task rc:before.rc ls start.after:5/1/2009};
unlike ($output, qr/foo/, 'no foo after 5/1/2009');
unlike ($output, qr/bar/, 'no bar after 5/1/2009');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'before.rc';
ok (!-r 'before.rc', 'Removed before.rc');

exit 0;

