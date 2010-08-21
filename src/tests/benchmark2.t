#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'bench2.rc')
{
  print $fh "data.location=.\n",
            "_forcecolor=1\n",
            "color.debug=\n",
            "debug=on\n";
  close $fh;
  ok (-r 'bench2.rc', 'Created bench2.rc');
}

# Data.
my @tags        = qw(t_one t_two t_three t_four t_five t_six t_seven t_eight);
my @projects    = qw(p_one p_two p_three p_foud p_five p_six p_seven p_eight);
my @priorities  = qw(H M L);
my $description = 'This is a medium-sized description with no special characters';

my $output;

# Add 1 task.
add (1);
$output = qx{../task rc:bench2.rc list};
report ('run-1', $output);

# Add 9 more tasks.
add (9);
$output = qx{../task rc:bench2.rc list};
report ('run-10', $output);

# Add 90 more tasks.
add (90);
$output = qx{../task rc:bench2.rc list};
report ('run-100', $output);

# Add 900 more tasks.
add (900);
$output = qx{../task rc:bench2.rc list};
report ('run-1000', $output);

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'bench2.rc';
ok (!-r 'bench2.rc', 'Removed bench2.rc');

exit 0;

################################################################################
sub add
{
  my ($quantity) = @_;

  for my $i (1 .. $quantity)
  {
    my $project  =   $projects[rand % 8];
    my $priority = $priorities[rand % 3];
    my $tag      =       $tags[rand % 8];

    qx{../task rc:bench2.rc add project:$project priority:$priority +$tag $i $description};
  }
}

################################################################################
sub report
{
  my ($label, $output) = @_;

  my %data;
  while ($output =~ /^Timer (\S+) ([0-9.e-]+)/msg)
  {
    $data{$1} += $2;
  }

  # Generate output for benchmark2 chart.
  chomp (my $version = qx{../task _version});
  my $out = sprintf "%s %s %f,%f,%f,%f,%f,%f,%f",
                    $label,
                    $version,
                    $data{'Context::initialize'},
                    $data{'Context::parse'},
                    $data{'TDB::loadPending'},
                    $data{'TDB::loadCompleted'} || 0,
                    $data{'TDB::gc'},
                    $data{'TDB::commit'},
                    $data{'Table::render'};

  diag ($out);
}

################################################################################

