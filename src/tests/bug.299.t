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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "confirmation=no\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Setup: Add three unique tasks with different project names.
qx{../task rc:bug.rc add project:one foo};
qx{../task rc:bug.rc add project:two bar};
qx{../task rc:bug.rc add project:three baz};

# Result: Run list but exclude two of the three projects names using
# project.hasnt:<name>
my $output = qx{../task rc:bug.rc list project.isnt:one project.isnt:two};
unlike ($output, qr/one.*foo/ms, 'project.isnt:one project.isnt:two - no foo');
unlike ($output, qr/two.*bar/ms, 'project.isnt:one project.isnt:two - no bar');
like   ($output, qr/three.*baz/ms, 'project.isnt:one project.isnt:two - yes baz');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'bug.rc';
ok (!-r 'bug.rc', 'Removed bug.rc');

exit 0;

