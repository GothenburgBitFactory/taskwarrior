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
use Test::More tests => 97;

# Create the rc file.
if (open my $fh, '>', 'sorting.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'sorting.rc', 'Created sorting.rc');
}

# Test assorted sort orders.
qx{../src/task rc:sorting.rc add                                    zero};
qx{../src/task rc:sorting.rc add priority:H project:A due:yesterday one};
qx{../src/task rc:sorting.rc add priority:M project:B due:today     two};
qx{../src/task rc:sorting.rc add priority:L project:C due:tomorrow  three};
qx{../src/task rc:sorting.rc add priority:H project:C due:today     four};

qx{../src/task rc:sorting.rc 2 start};
qx{../src/task rc:sorting.rc 2 start};

my %tests =
(
  # Single sort column.
  'priority-'                       => '(?:one.+four|four.+one).+two.+three.+zero',
  'priority+'                       => 'zero.+three.+two.+(?:one.+four|four.+one)',
  'project-'                        => '(?:three.+four|four.+three).+two.+one.+zero',
  'project+'                        => 'zero.+one.+two.+(?:three.+four|four.+three)',
  'start-'                          => '(?:one.+three|three.+one).+(?:zero.+two.+four|zero.+four.+two|two.+zero.+four|two.+four.+zero|four.+zero.+two|four.+two.+zero)',
  'start+'                          => '(?:zero.+two.+four|zero.+four.+two|two.+zero.+four|two.+four.+zero|four.+zero.+two|four.+two.+zero).+(?:one.+three|three.+one)',
  'due-'                            => 'three.+(?:two.+four|four.+two).+one.+zero',
  'due+'                            => 'one.+(?:two.+four|four.+two).+three.+zero',
  'description-'                    => 'zero.+two.+three.+one.+four',
  'description+'                    => 'four.+one.+three.+two.+zero',

  # Two sort columns.
  'priority-,project-'              => 'four.+one.+two.+three.+zero',
  'priority-,project+'              => 'one.+four.+two.+three.+zero',
  'priority+,project-'              => 'zero.+three.+two.+four.+one',
  'priority+,project+'              => 'zero.+three.+two.+one.+four',

  'priority-,start-'                => 'one.+four.+two.+three.+zero',
  'priority-,start+'                => 'four.+one.+two.+three.+zero',
  'priority+,start-'                => 'zero.+three.+two.+one.+four',
  'priority+,start+'                => 'zero.+three.+two.+four.+one',

  'priority-,due-'                  => 'four.+one.+two.+three.+zero',
  'priority-,due+'                  => 'one.+four.+two.+three.+zero',
  'priority+,due-'                  => 'zero.+three.+two.+four.+one',
  'priority+,due+'                  => 'zero.+three.+two.+one.+four',

  'priority-,description-'          => 'one.+four.+two.+three.+zero',
  'priority-,description+'          => 'four.+one.+two.+three.+zero',
  'priority+,description-'          => 'zero.+three.+two.+one.+four',
  'priority+,description+'          => 'zero.+three.+two.+four.+one',

  'project-,priority-'              => 'four.+three.+two.+one.+zero',
  'project-,priority+'              => 'three.+four.+two.+one.+zero',
  'project+,priority-'              => 'zero.+one.+two.+four.+three',
  'project+,priority+'              => 'zero.+one.+two.+three.+four',

  'project-,start-'                 => 'three.+four.+two.+one.+zero',
  'project-,start+'                 => 'four.+three.+two.+one.+zero',
  'project+,start-'                 => 'zero.+one.+two.+three.+four',
  'project+,start+'                 => 'zero.+one.+two.+four.+three',

  'project-,due-'                   => 'three.+four.+two.+one.+zero',
  'project-,due+'                   => 'four.+three.+two.+one.+zero',
  'project+,due-'                   => 'zero.+one.+two.+three.+four',
  'project+,due+'                   => 'zero.+one.+two.+four.+three',

  'project-,description-'           => 'three.+four.+two.+one.+zero',
  'project-,description+'           => 'four.+three.+two.+one.+zero',
  'project+,description-'           => 'zero.+one.+two.+three.+four',
  'project+,description+'           => 'zero.+one.+two.+four.+three',

  'start-,priority-'                => 'one.+three.+four.+two.+zero',
  'start-,priority+'                => 'three.+one.+zero.+two.+four',
  'start+,priority-'                => 'four.+two.+zero.+one.+three',
  'start+,priority+'                => 'zero.+two.+four.+three.+one',

  'start-,project-'                 => 'three.+one.+four.+two.+zero',
  'start-,project+'                 => 'one.+three.+zero.+two.+four',
  'start+,project-'                 => 'four.+two.+zero.+three.+one',
  'start+,project+'                 => 'zero.+two.+four.+one.+three',

  'start-,due-'                     => 'three.+one.+(?:four.+two|two.+four).+zero',
  'start-,due+'                     => 'one.+three.+(?:four.+two|two.+four).+zero',
  'start+,due-'                     => '(?:four.+two|two.+four).+zero.+three.+one',
  'start+,due+'                     => '(?:four.+two|two.+four).+zero.+one.+three',

  'start-,description-'             => 'three.+one.+zero.+two.+four',
  'start-,description+'             => 'one.+three.+four.+two.+zero',
  'start+,description-'             => 'zero.+two.+four.+three.+one',
  'start+,description+'             => 'four.+two.+zero.+one.+three',

  'due-,priority-'                  => 'three.+four.+two.+one.+zero',
  'due-,priority+'                  => 'three.+two.+four.+one.+zero',
  'due+,priority-'                  => 'one.+four.+two.+three.+zero',
  'due+,priority+'                  => 'one.+two.+four.+three.+zero',

  'due-,project-'                   => 'three.+four.+two.+one.+zero',
  'due-,project+'                   => 'three.+two.+four.+one.+zero',
  'due+,project-'                   => 'one.+four.+two.+three.+zero',
  'due+,project+'                   => 'one.+two.+four.+three.+zero',

  'due-,start-'                     => 'three.+(?:four.+two|two.+four).+one.+zero',
  'due-,start+'                     => 'three.+(?:four.+two|two.+four).+one.+zero',
  'due+,start-'                     => 'one.+(?:four.+two|two.+four).+three.+zero',
  'due+,start+'                     => 'one.+(?:four.+two|two.+four).+three.+zero',

  'due-,description-'               => 'three.+two.+four.+one.+zero',
  'due-,description+'               => 'three.+four.+two.+one.+zero',
  'due+,description-'               => 'one.+two.+four.+three.+zero',
  'due+,description+'               => 'one.+four.+two.+three.+zero',

  'description-,priority-'          => 'zero.+two.+three.+one.+four',
  'description-,priority+'          => 'zero.+two.+three.+one.+four',
  'description+,priority-'          => 'four.+one.+three.+two.+zero',
  'description+,priority+'          => 'four.+one.+three.+two.+zero',

  'description-,project-'           => 'zero.+two.+three.+one.+four',
  'description-,project+'           => 'zero.+two.+three.+one.+four',
  'description+,project-'           => 'four.+one.+three.+two.+zero',
  'description+,project+'           => 'four.+one.+three.+two.+zero',

  'description-,start-'             => 'zero.+two.+three.+one.+four',
  'description-,start+'             => 'zero.+two.+three.+one.+four',
  'description+,start-'             => 'four.+one.+three.+two.+zero',
  'description+,start+'             => 'four.+one.+three.+two.+zero',

  'description-,due-'               => 'zero.+two.+three.+one.+four',
  'description-,due+'               => 'zero.+two.+three.+one.+four',
  'description+,due-'               => 'four.+one.+three.+two.+zero',
  'description+,due+'               => 'four.+one.+three.+two.+zero',

  # Four sort columns.
  'start+,project+,due+,priority+'  => 'zero.+two.+four.+one.+three',
  'project+,due+,priority+,start+'  => 'zero.+one.+two.+four.+three',
);

for my $sort (sort keys %tests)
{
  my $output = qx{../src/task rc:sorting.rc rc.report.list.sort:${sort} list};
  like ($output, qr/$tests{$sort}/ms, "sort:${sort}");
}

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'sorting.rc';
ok (!-r 'sorting.rc', 'Removed sorting.rc');

exit 0;

