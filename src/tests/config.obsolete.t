#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
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
use Test::More tests => 6;

# Create the rc file.
if (open my $fh, '>', 'obsolete.rc')
{
  print $fh "data.location=.\n",
            "foo=1\n";
  close $fh;
  ok (-r 'obsolete.rc', 'Created obsolete.rc');
}

# Test the add command.
my $output = qx{../task rc:obsolete.rc version};

like ($output, qr/Your .taskrc file contains these unrecognized variables:\n/,
               'unsupported configuration variable');
like ($output, qr/  foo\n/, 'unsupported configuration variable');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'obsolete.rc';
ok (!-r 'obsolete.rc', 'Removed obsolete.rc');

exit 0;

