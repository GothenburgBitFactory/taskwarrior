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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'bench.rc')
{
  print $fh "data.location=.\n",
            "_forcecolor=1\n";
  close $fh;
  ok (-r 'bench.rc', 'Created bench.rc');
}

# Do lots of things.  Time it all.

my @tags        = qw(t_one t_two t_three t_four t_five t_six t_seven t_eight);
my @projects    = qw(p_one p_two p_three p_foud p_five p_six p_seven p_eight);
my @priorities  = qw(H M L);
my $description = 'This is a medium-sized description with no special characters';

# Start the clock.
my $start = time ();
my $cursor = $start;
diag ("start=$start");

# Make a mess.
for my $i (1 .. 1000)
{
  my $project  =   $projects[rand % 8];
  my $priority = $priorities[rand % 3];
  my $tag      =       $tags[rand % 8];

  qx{../src/task rc:bench.rc add project:$project priority:$priority +$tag $i $description};
}
diag ("1000 tasks added in " . (time () - $cursor) . " seconds");
$cursor = time ();

qx{../src/task rc:bench.rc modify /with/WITH/}  for   1 .. 200;
qx{../src/task rc:bench.rc $_ done}             for 201 .. 400;
qx{../src/task rc:bench.rc $_ start}            for 401 .. 600;
diag ("600 tasks altered in " . (time () - $cursor) . " seconds");
$cursor = time ();

# Report it all.  Note that all Timer information is displayed.

for (1 .. 100)
{
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc ls});
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc list});
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc list priority:H});
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc list +tag});
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc list project_A});
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc long});
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc completed});
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc history});
  diag (grep {/^Timer /} qx{../src/task rc:bench.rc ghistory});
}

# Stop the clock.
my $stop = time ();
diag ("stop=$stop");
diag ("total=" . ($stop - $start));

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'bench.rc';
ok (!-r 'bench.rc', 'Removed bench.rc');

exit 0;

