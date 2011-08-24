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
use Test::More tests => 11;

# Create the rc file.
if (open my $fh, '>', '440.rc')
{
  print $fh "data.location=.";

  close $fh;
  ok (-r '440.rc', 'Created 440.rc');
}

# Bug #440: Parser recognizes an attempt to simultaneously subst and append, but doesn't do it

# Create a task and attempt simultaneous subst and appends, both permutations

qx{../src/task rc:440.rc add Foo};
qx{../src/task rc:440.rc add Foo};

qx{../src/task rc:440.rc 1 append /Foo/Bar/ Appendtext};
qx{../src/task rc:440.rc 2 append Appendtext /Foo/Bar/};

my $output1 = qx{../src/task rc:440.rc 1 ls};
my $output2 = qx{../src/task rc:440.rc 2 ls};

unlike ($output1, qr/Foo/, 'simultaneous subst and append - subst');
like ($output1, qr/\w+ Appendtext/, 'simultaneous subst and append - append');

unlike ($output2, qr/Foo/, 'simultaneous append and subst - subst');
like ($output2, qr/\w+ Appendtext/, 'simultaneous append and subst - append');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'backlog.data';
ok (!-r 'backlog.data', 'Removed backlog.data');

unlink 'synch.key';
ok (!-r 'synch.key', 'Removed synch.key');

unlink '440.rc';
ok (!-r '440.rc', 'Removed 440.rc');

exit 0;
