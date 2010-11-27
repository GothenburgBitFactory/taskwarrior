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
use Test::More tests => 7;

# Create the rc file.
if (open my $fh, '>', 'hang.rc')
{
  print $fh "data.location=.\n",
            "shadow.file=shadow.txt\n",
            "shadow.command=list\n";
  close $fh;
  ok (-r 'hang.rc', 'Created hang.rc');
}

=pod
I found a bug in the current version of task. Using recur and a shadow file will
lead to an infinite loop. To reproduce it, define a shadow file in the .taskrc,
set a command for it that rebuilds the database, e.g. "list", and then add a
task with a recurrence set, e.g. "task add due:today recur:1d infinite loop".
Task will then loop forever and add the same recurring task until it runs out of
memory. So I checked the source and I believe I found the cause.
handleRecurrence() in task.cpp will modify the mask, but writes it only after it
has added all new tasks. Adding the task will, however, invoke onChangeCallback,
which starts the same process all over again.
=cut

eval
{
  $SIG{'ALRM'} = sub {die "alarm\n"};
  alarm 10;
  my $output = qx{../task rc:hang.rc list;
                  ../task rc:hang.rc add due:today recur:1d infinite loop;
                  ../task rc:hang.rc info 1};
  alarm 0;

  like ($output, qr/^Description\s+infinite loop\n/m, 'no hang');
};

if ($@ eq "alarm\n")
{
  fail ('task hang on add or recurring task, with shadow file, for 10s');
}

# Cleanup.
unlink 'shadow.txt';
ok (!-r 'shadow.txt', 'Removed shadow.txt');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'hang.rc';
ok (!-r 'hang.rc', 'Removed hang.rc');

exit 0;

