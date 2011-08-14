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
use Test::More tests => 24;

# Create the rc file.
if (open my $fh, '>', 'shadow.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "shadow.file=./shadow.txt\n",
            "shadow.command=rc:shadow.rc stats\n",
            "shadow.notify=on\n";
  close $fh;
  ok (-r 'shadow.rc', 'Created shadow.rc');
}

my $output = qx{../src/task rc:shadow.rc add one};
like ($output, qr/\[Shadow file '\.\/shadow\.txt' updated\.\]/, 'shadow file updated on add');

$output = qx{../src/task rc:shadow.rc list};
unlike ($output, qr/\[Shadow file '\.\/shadow\.txt' updated\.\]/, 'shadow file not updated on list');

$output = qx{../src/task rc:shadow.rc 1 delete};
like ($output, qr/\[Shadow file '\.\/shadow\.txt' updated\.\]/, 'shadow file updated on delete');

$output = qx{../src/task rc:shadow.rc list};
unlike ($output, qr/\[Shadow file '\.\/shadow\.txt' updated\.\]/, 'shadow file not updated on list');

# Inspect the shadow file.
my $file = slurp ('./shadow.txt');
like ($file, qr/Pending\s+0\n/,                        'Pending 0');
like ($file, qr/Recurring\s+0\n/,                      'Recurring 0');
like ($file, qr/Completed\s+0\n/,                      'Completed 0');
like ($file, qr/Deleted\s+1\n/,                        'Deleted 1');
like ($file, qr/Total\s+1\n/,                          'Total 1');
like ($file, qr/Task used for\s+-\n/,                  'Task used for -');
like ($file, qr/Task added every\s+-\n/,               'Task added every -');
like ($file, qr/Task deleted every\s+-\n/,             'Task deleted every -');
like ($file, qr/Average desc length\s+3 characters\n/, 'Average desc length 3 characters');
like ($file, qr/Tasks tagged\s+0%\n/,                  'Tasks tagged 0%');
like ($file, qr/Unique tags\s+0\n/,                    'Unique tags 0');
like ($file, qr/Projects\s+0\n/,                       'Projects 0');

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

unlink 'shadow.rc';
ok (!-r 'shadow.rc', 'Removed shadow.rc');

unlink 'shadow.txt';
ok (!-r 'shadow.txt', 'Removed shadow.txt');

exit 0;

################################################################################
sub slurp
{
  my ($file) = @_;
  local $/;

  if (open my $fh, '<', $file)
  {
    my $contents = <$fh>;
    close $fh;
    return $contents;
  }

  '';
}

