#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
if (open my $fh, '>', 'color.rc')
{
  print $fh "data.location=.\n",
            "search.case.sensitive=yes\n",
            "color=on\n",
            "color.alternate=\n",
            "color.keyword.red=red\n",
            "color.keyword.green=green\n",
            "color.keyword.yellow=yellow\n",
            "_forcecolor=1\n";
  close $fh;
  ok (-r 'color.rc', 'Created color.rc');
}

# Test the add command.
qx{../src/task rc:color.rc add nothing};
qx{../src/task rc:color.rc add red};
qx{../src/task rc:color.rc add green};
qx{../src/task rc:color.rc add -- annotation};
qx{../src/task rc:color.rc 4 annotate yellow};
my $output = qx{../src/task rc:color.rc list};

like ($output, qr/ (?!<\033\[\d\dm) .* nothing    .* (?!>\033\[0m) /x, 'none');
like ($output, qr/ \033\[31m        .* red        .* \033\[0m      /x, 'color.keyword.red');
like ($output, qr/ \033\[32m        .* green      .* \033\[0m      /x, 'color.keyword.green');
like ($output, qr/ \033\[33m        .* annotation .* \033\[0m      /x, 'color.keyword.yellow (annotation)');

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

