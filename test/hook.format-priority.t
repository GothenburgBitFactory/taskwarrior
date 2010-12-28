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
use Test::More tests => 10;

# Create the rc file.
if (open my $fh, '>', 'hook.rc')
{
  print $fh "data.location=.\n",
            "hooks=on\n",
            "hook.format-priority=" . $ENV{'PWD'} . "/hook:priority\n";
  close $fh;
  ok (-r 'hook.rc', 'Created hook.rc');
}

# Create the hook functions.
if (open my $fh, '>', 'hook')
{
  print $fh "function priority (name, value)\n",
            "  if value == 'H' then\n",
            "    value = 'Hi'\n",
            "  elseif value == 'M' then\n",
            "    value = 'Me'\n",
            "  elseif value == 'L' then\n",
            "    value = 'Lo'\n",
            "  end\n",
            "  return value, 0, nil\n",
            "end\n";
  close $fh;
  ok (-r 'hook', 'Created hook');
}

my $output = qx{../src/task rc:hook.rc version};
if ($output =~ /PUC-Rio/)
{
  qx{../src/task rc:hook.rc add foo pri:H};
  qx{../src/task rc:hook.rc add bar pri:M};
  qx{../src/task rc:hook.rc add baz pri:L};
  $output = qx{../src/task rc:hook.rc ls};

  like ($output, qr/Hi\s+foo/, 'format-priority hook H -> Hi');
  like ($output, qr/Me\s+bar/, 'format-priority hook M -> Me');
  like ($output, qr/Lo\s+baz/, 'format-priority hook L -> Lo');
}
else
{
  pass ('format-priority hook H -> Hi - skip: no Lua support');
  pass ('format-priority hook M -> Me - skip: no Lua support');
  pass ('format-priority hook L -> Lo - skip: no Lua support');
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

