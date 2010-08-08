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
use Test::More tests => 8;

# Create the rc file.
if (open my $fh, '>', 'hook.rc')
{
  print $fh "data.location=.\n",
            "hooks=on\n",
            "hook.pre-display=" . $ENV{'PWD'} . "/hook:test\n";
  close $fh;
  ok (-r 'hook.rc', 'Created hook.rc');
}

if (open my $fh, '>', 'hook')
{
  print $fh "function test ()\n",
            "  print ('<<' .. task_get_until () .. '>>')\n",
            "  return 0, nil\n",
            "end\n";
  close $fh;
  ok (-r 'hook', 'Created hook');
}

my $output = qx{../task rc:hook.rc version};
if ($output =~ /PUC-Rio/)
{
  # Test the hook.
  qx{../task rc:hook.rc add foo due:tomorrow recur:weekly until:eoy};
  $output = qx{../task rc:hook.rc info 1};
  like ($output, qr/^<<\d+>>$/ms, 'Hook called task_get_until');
}
else
{
  pass ('Hook called task_get_until - skip: no Lua support');
}

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'hook';
ok (!-r 'hook', 'Removed hook');

unlink 'hook.rc';
ok (!-r 'hook.rc', 'Removed hook.rc');

exit 0;

