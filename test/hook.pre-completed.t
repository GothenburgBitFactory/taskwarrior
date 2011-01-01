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
if (open my $fh, '>', 'hook.rc')
{
  print $fh "data.location=.\n",
            "hooks=on\n";
  close $fh;
  ok (-r 'hook.rc', 'Created hook.rc');
}

# Create the hook functions.
if (open my $fh, '>', 'hook')
{
  print $fh "function good () print ('marker') return 0, nil end\n",
            "function  bad () print ('marker') return 1, 'disallowed' end\n";
  close $fh;
  ok (-r 'hook', 'Created hook');
}

my $output = qx{../src/task rc:hook.rc version};
if ($output =~ /PUC-Rio/)
{
  my $good = $ENV{'PWD'} . '/hook:good';
  my $bad  = $ENV{'PWD'} . '/hook:bad';

  qx{echo 'y'|../src/task rc:hook.rc config -- hook.pre-completed "$bad"};
  qx{../src/task rc:hook.rc add foo};
  $output = qx{../src/task rc:hook.rc done 1};
  like ($output, qr/disallowed/, 'pre-completed hook rejected completion');

  qx{echo 'y'|../src/task rc:hook.rc config -- hook.pre-completed "$good"};
  $output = qx{../src/task rc:hook.rc done 1};
  like ($output, qr/Marked 1 task as done/, 'pre-completed hook allowed completion');
}
else
{
  pass ('pre-complete hook rejected completion - skip: no Lua support');
  pass ('pre-complete hook allowed completion - skip: no Lua support');
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

