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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'shell.rc')
{
  print $fh "data.location=.\n",
            "shell.prompt=testprompt>\n",
            "default.command=ls\n";
  close $fh;
  ok (-r 'shell.rc', 'Created shell.rc');
}

# Test the prompt.
my $output = qx{echo "-- \\nquit\\n" | ../src/task rc:shell.rc shell};
like ($output, qr/testprompt>/, 'custom prompt is being used');

# Test a simple add, then info.
$output = qx{echo "-- add foo\ninfo 1\n" | ../src/task rc:shell.rc shell};
like ($output, qr/Description\s+foo/, 'add/info working');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'response.txt';
ok (!-r 'response.txt', 'Removed response.txt');

unlink 'shell.rc';
ok (!-r 'shell.rc', 'Removed shell.rc');

exit 0;

