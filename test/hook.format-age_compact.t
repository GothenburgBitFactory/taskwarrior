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
if (open my $fh, '>', 'hook.rc')
{
  print $fh "data.location=.\n",
            "hooks=on\n",
            "hook.format-age_compact=" . $ENV{'PWD'} . "/hook:age\n",
            "report.list.columns=id,project,priority,due,active,age_compact,description\n";
  close $fh;
  ok (-r 'hook.rc', 'Created hook.rc');
}

# Create the hook functions.
if (open my $fh, '>', 'hook')
{
  print $fh "function age (name, value)\n",
            "  value = '<' .. value .. '>'\n",
            "  return value, 0, nil\n",
            "end\n";
  close $fh;
  ok (-r 'hook', 'Created hook');
}

my $output = qx{../src/task rc:hook.rc version};
if ($output =~ /PUC-Rio/)
{
  qx{../src/task rc:hook.rc add foo};
  diag ("1 second delay");
  sleep 1;
  $output = qx{../src/task rc:hook.rc list};

  like ($output, qr/<\ds>/, 'format-age_compact hook age -> <age>');
}
else
{
  pass ('format-age_compact hook age -> <age> - skip: no Lua support');
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

