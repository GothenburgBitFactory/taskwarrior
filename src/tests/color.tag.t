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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'color.rc')
{
  print $fh "data.location=.\n",
            "color.tagged=\n",
            "color.alternate=\n",
            "color.tag.red=red\n",
            "color.tag.green=green\n",
            "_forcecolor=1\n";
  close $fh;
  ok (-r 'color.rc', 'Created color.rc');
}

# Test the add command.
qx{../task rc:color.rc add nothing};
qx{../task rc:color.rc add +red red};
qx{../task rc:color.rc add +green green};
my $output = qx{../task rc:color.rc list};

like ($output, qr/ (?!<\033\[\d\dm) .* nothing .* (?!>\033\[0m) /x, 'none');
like ($output, qr/ \033\[31m        .* red     .* \033\[0m /x, 'color.tag.red');
like ($output, qr/ \033\[32m        .* green   .* \033\[0m /x, 'color.tag.green');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'color.rc';
ok (!-r 'color.rc', 'Removed color.rc');

exit 0;

