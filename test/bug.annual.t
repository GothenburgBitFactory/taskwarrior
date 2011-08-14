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
use Test::More tests => 17;

# Create the rc file.
if (open my $fh, '>', 'annual.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'annual.rc', 'Created annual.rc');
}

# If a task is added with a due date ten years ago, with an annual recurrence,
# then the synthetic tasks in between then and now have a due date that creeps.
#
# ID Project Pri Due        Active Age Description
# -- ------- --- ---------- ------ --- -----------
#  2               1/1/2000          - foo
#  3             12/31/2000          - foo
#  4             12/31/2001          - foo
#  5             12/31/2002          - foo
#  6             12/31/2003          - foo
#  7             12/30/2004          - foo
#  8             12/30/2005          - foo
#  9             12/30/2006          - foo
# 10             12/30/2007          - foo
# 11             13/29/2008          - foo
# 12             12/29/2009          - foo

qx{../src/task rc:annual.rc add foo due:1/1/2000 recur:annual until:1/1/2009};
my $output = qx{../src/task rc:annual.rc list};
like ($output, qr/2\s+1\/1\/2000\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 2 no creep');
like ($output, qr/3\s+1\/1\/2001\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 3 no creep');
like ($output, qr/4\s+1\/1\/2002\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 4 no creep');
like ($output, qr/5\s+1\/1\/2003\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 5 no creep');
like ($output, qr/6\s+1\/1\/2004\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 6 no creep');
like ($output, qr/7\s+1\/1\/2005\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 7 no creep');
like ($output, qr/8\s+1\/1\/2006\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 8 no creep');
like ($output, qr/9\s+1\/1\/2007\s+(?:-|\d+\ssecs?)\s+foo/,  'synthetic 9 no creep');
like ($output, qr/10\s+1\/1\/2008\s+(?:-|\d+\ssecs?)\s+foo/, 'synthetic 10 no creep');
like ($output, qr/11\s+1\/1\/2009\s+(?:-|\d+\ssecs?)\s+foo/, 'synthetic 11 no creep');

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

unlink 'annual.rc';
ok (!-r 'annual.rc', 'Removed annual.rc');

exit 0;

