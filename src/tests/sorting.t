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
use Test::More tests => 97;

# Create the rc file.
if (open my $fh, '>', 'sorting.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'sorting.rc', 'Created sorting.rc');
}

# Test assorted sort orders.

qx{../task rc:sorting.rc add                                    zero};
qx{../task rc:sorting.rc add priority:H project:A due:yesterday one};
qx{../task rc:sorting.rc add priority:M project:B due:today     two};
qx{../task rc:sorting.rc add priority:L project:C due:tomorrow  three};
qx{../task rc:sorting.rc add priority:H project:C due:today     four};

qx{../task rc:sorting.rc start 2};
qx{../task rc:sorting.rc start 4};

# pri:H pro:C   due:today     four
# pri:H pro:A * due:yesterday one
# pri:M pro:B   due:today     two
# pri:L pro:C * due:tomorrow  three
#                             zero

my %tests =
(
  # Single sort column.
  'priority-'                       => '(?:one.+four|four.+one).+two.+three.+zero',
  'priority+'                       => 'zero.+three.+two.+(?:one.+four|four.+one)',
  'project-'                        => '(?:three.+four|four.+three).+two.+one.+zero',
  'project+'                        => 'zero.+one.+two.+(?:three.+four|four.+three)',
  'active-'                         => '(?:one.+three|three.+one).+(?:zero.+two.+four|zero.+four.+two|two.+zero.+four|two.+four.+zero|four.+zero.+two|four.+two.+zero)',
  'active+'                         => '(?:zero.+two.+four|zero.+four.+two|two.+zero.+four|two.+four.+zero|four.+zero.+two|four.+two.+zero).+(?:one.+three|three.+one)',
  'due-'                            => 'three.+(?:two.+four|four.+two).+one.+zero',
  'due+'                            => 'one.+(?:two.+four|four.+two).+three.+zero',
  'description-'                    => 'zero.+two.+three.+one.+four',
  'description+'                    => 'four.+one.+three.+two.+zero',

  # Two sort columns.
  'priority-,project-'              => 'four.+one.+two.+three.+zero',
  'priority-,project+'              => 'one.+four.+two.+three.+zero',
  'priority+,project-'              => 'zero.+three.+two.+four.+one',
  'priority+,project+'              => 'zero.+three.+two.+one.+four',

  'priority-,active-'               => 'one.+four.+two.+three.+zero',
  'priority-,active+'               => 'four.+one.+two.+three.+zero',
  'priority+,active-'               => 'zero.+three.+two.+one.+four',
  'priority+,active+'               => 'zero.+three.+two.+four.+one',

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

  'project-,active-'                => 'three.+four.+two.+one.+zero',
  'project-,active+'                => 'four.+three.+two.+one.+zero',
  'project+,active-'                => 'zero.+one.+two.+three.+four',
  'project+,active+'                => 'zero.+one.+two.+four.+three',

  'project-,due-'                   => 'three.+four.+two.+one.+zero',
  'project-,due+'                   => 'four.+three.+two.+one.+zero',
  'project+,due-'                   => 'zero.+one.+two.+three.+four',
  'project+,due+'                   => 'zero.+one.+two.+four.+three',

  'project-,description-'           => 'three.+four.+two.+one.+zero',
  'project-,description+'           => 'four.+three.+two.+one.+zero',
  'project+,description-'           => 'zero.+one.+two.+three.+four',
  'project+,description+'           => 'zero.+one.+two.+four.+three',

  'active-,priority-'               => 'one.+three.+four.+two.+zero',
  'active-,priority+'               => 'three.+one.+zero.+two.+four',
  'active+,priority-'               => 'four.+two.+zero.+one.+three',
  'active+,priority+'               => 'zero.+two.+four.+three.+one',

  'active-,project-'                => 'three.+one.+four.+two.+zero',
  'active-,project+'                => 'one.+three.+zero.+two.+four',
  'active+,project-'                => 'four.+two.+zero.+three.+one',
  'active+,project+'                => 'zero.+two.+four.+one.+three',

  'active-,due-'                    => 'three.+one.+(?:four.+two|two.+four).+zero',
  'active-,due+'                    => 'one.+three.+(?:four.+two|two.+four).+zero',
  'active+,due-'                    => '(?:four.+two|two.+four).+zero.+three.+one',
  'active+,due+'                    => '(?:four.+two|two.+four).+zero.+one.+three',

  'active-,description-'            => 'three.+one.+zero.+two.+four',
  'active-,description+'            => 'one.+three.+four.+two.+zero',
  'active+,description-'            => 'zero.+two.+four.+three.+one',
  'active+,description+'            => 'four.+two.+zero.+one.+three',

  'due-,priority-'                  => 'three.+four.+two.+one.+zero',
  'due-,priority+'                  => 'three.+two.+four.+one.+zero',
  'due+,priority-'                  => 'one.+four.+two.+three.+zero',
  'due+,priority+'                  => 'one.+two.+four.+three.+zero',

  'due-,project-'                   => 'three.+four.+two.+one.+zero',
  'due-,project+'                   => 'three.+two.+four.+one.+zero',
  'due+,project-'                   => 'one.+four.+two.+three.+zero',
  'due+,project+'                   => 'one.+two.+four.+three.+zero',

  'due-,active-'                    => 'three.+(?:four.+two|two.+four).+one.+zero',
  'due-,active+'                    => 'three.+(?:four.+two|two.+four).+one.+zero',
  'due+,active-'                    => 'one.+(?:four.+two|two.+four).+three.+zero',
  'due+,active+'                    => 'one.+(?:four.+two|two.+four).+three.+zero',

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

  'description-,active-'            => 'zero.+two.+three.+one.+four',
  'description-,active+'            => 'zero.+two.+three.+one.+four',
  'description+,active-'            => 'four.+one.+three.+two.+zero',
  'description+,active+'            => 'four.+one.+three.+two.+zero',

  'description-,due-'               => 'zero.+two.+three.+one.+four',
  'description-,due+'               => 'zero.+two.+three.+one.+four',
  'description+,due-'               => 'four.+one.+three.+two.+zero',
  'description+,due+'               => 'four.+one.+three.+two.+zero',

  # Four sort columns.
  'active+,project+,due+,priority+' => 'zero.+two.+four.+one.+three',
  'project+,due+,priority+,active+' => 'zero.+one.+two.+four.+three',
);

for my $sort (sort keys %tests)
{
  my $output = qx{../task rc:sorting.rc rc.report.list.sort:${sort} list};
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

