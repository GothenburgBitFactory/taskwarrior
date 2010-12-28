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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'color.rc')
{
  print $fh "data.location=.\n",
            "color.pri.H=red on black\n",
            "color.pri.M=red on_black\n";
  close $fh;
  ok (-r 'color.rc', 'Created color.rc');
}

# Test the add command.
my $output = qx{../task rc:color.rc show};
like   ($output, qr/that use deprecated underscores/ms,  'Deprecated color detected');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'color.rc';
ok (!-r 'color.rc', 'Removed color.rc');

exit 0;

