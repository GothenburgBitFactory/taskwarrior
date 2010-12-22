#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 9;

# Create the rc file.
if (open my $fh, '>', 'limit.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'limit.rc', 'Created limit.rc');
}

# Add a large number of tasks (> 25).
qx{../task rc:limit.rc add one};
qx{../task rc:limit.rc add two};
qx{../task rc:limit.rc add three};
qx{../task rc:limit.rc add four};
qx{../task rc:limit.rc add five};
qx{../task rc:limit.rc add six};
qx{../task rc:limit.rc add seven};
qx{../task rc:limit.rc add eight};
qx{../task rc:limit.rc add nine};
qx{../task rc:limit.rc add ten};
qx{../task rc:limit.rc add eleven};
qx{../task rc:limit.rc add twelve};
qx{../task rc:limit.rc add thirteen};
qx{../task rc:limit.rc add fourteen};
qx{../task rc:limit.rc add fifteen};
qx{../task rc:limit.rc add sixteen};
qx{../task rc:limit.rc add seventeen};
qx{../task rc:limit.rc add eighteen};
qx{../task rc:limit.rc add nineteen};
qx{../task rc:limit.rc add twenty};
qx{../task rc:limit.rc add twenty one};
qx{../task rc:limit.rc add twenty two};
qx{../task rc:limit.rc add twenty three};
qx{../task rc:limit.rc add twenty four};
qx{../task rc:limit.rc add twenty five};
qx{../task rc:limit.rc add twenty six};
qx{../task rc:limit.rc add twenty seven};
qx{../task rc:limit.rc add twenty eight};
qx{../task rc:limit.rc add twenty nine};
qx{../task rc:limit.rc add thirty};

my $output = qx{../task rc:limit.rc ls};
like ($output, qr/^30 tasks$/ms, 'unlimited');

$output = qx{../task rc:limit.rc ls limit:0};
like ($output, qr/^30 tasks$/ms, 'limited to 0 - unlimited');

$output = qx{../task rc:limit.rc ls limit:3};
like ($output, qr/^30 tasks, 3 shown$/ms, 'limited to 3');

$output = qx{../task rc:limit.rc ls limit:page};
like ($output, qr/^30 tasks, truncated to 17 tasks$/ms, 'limited to page');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'limit.rc';
ok (!-r 'limit.rc', 'Removed limit.rc');

exit 0;

