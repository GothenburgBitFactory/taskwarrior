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
            "report.long.columns=id,project,priority,entry,start,due,",
              "recur,countdown_compact,age,depends,tags,description\n",
            "hooks=on\n",
            "hook.format-countdown_compact=" . $ENV{'PWD'} . "/hook:countdown_compact\n";
  close $fh;
  ok (-r 'hook.rc', 'Created hook.rc');
}

# Create the hook functions.
if (open my $fh, '>', 'hook')
{
  print $fh "function countdown_compact (name, value)\n",
            "  value = '<' .. value .. '>'\n",
            "  return value, 0, nil\n",
            "end\n";
  close $fh;
  ok (-r 'hook', 'Created hook');
}

my $output = qx{../task rc:hook.rc version};
if ($output =~ /PUC-Rio/)
{
  qx{../task rc:hook.rc add foo due:eom};
  $output = qx{../task rc:hook.rc long};

  like ($output, qr/<-?\d+\D+>/, 'format-countdown_compact hook countdown_compact -> <countdown_compact>');
}
else
{
  pass ('format-countdown_compact hook countdown_compact -> <countdown_compact> - skip: no Lua support');
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

