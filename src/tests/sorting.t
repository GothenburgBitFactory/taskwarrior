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
use Test::More tests => 97;

# Create the rc file.
if (open my $fh, '>', 'sorting.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'sorting.rc', 'Created sorting.rc');
}

# Test assorted sort orders.

qx{../task rc:sorting.rc add priority:H project:A due:yesterday one};
qx{../task rc:sorting.rc add priority:M project:B due:today     two};
qx{../task rc:sorting.rc add priority:L project:C due:tomorrow  three};
qx{../task rc:sorting.rc add priority:H project:C due:today     four};

qx{../task rc:sorting.rc start 1};
qx{../task rc:sorting.rc start 3};

# pri:H pro:C   due:today     four
# pri:H pro:A * due:yesterday one
# pri:M pro:B   due:today     two
# pri:L pro:C * due:tomorrow  three

my %tests =
(
  # Single sort column.
  'priority-'                       => '(?:one.+four|four.+one).+two.+three',
  'priority+'                       => 'three.+two.+(?:one.+four|four.+one)',
  'project-'                        => '(?:three.+four|four.+three).+two.+one',
  'project+'                        => 'one.+two.+(?:three.+four|four.+three)',
  'active-'                         => '(?:one.+three|three.+one).+(?:two.+four|four.+two)',
  'active+'                         => '(?:two.+four|four.+two).+(?:one.+three|three.+one)',
  'due-'                            => 'three.+(?:two.+four|four.+two).+one',
  'due+'                            => 'one.+(?:two.+four|four.+two).+three',
  'description-'                    => 'two.+three.+one.+four',
  'description+'                    => 'four.+one.+three.+two',

  # Two sort columns.
  'priority-,project-'              => 'four.+one.+two.+three',
  'priority-,project+'              => 'one.+four.+two.+three',
  'priority+,project-'              => 'three.+two.+four.+one',
  'priority+,project+'              => 'three.+two.+one.+four',

  'priority-,active-'               => 'one.+four.+two.+three',
  'priority-,active+'               => 'four.+one.+two.+three',
  'priority+,active-'               => 'three.+two.+one.+four',
  'priority+,active+'               => 'three.+two.+four.+one',

  'priority-,due-'                  => 'four.+one.+two.+three',
  'priority-,due+'                  => 'one.+four.+two.+three',
  'priority+,due-'                  => 'three.+two.+four.+one',
  'priority+,due+'                  => 'three.+two.+one.+four',

  'priority-,description-'          => 'one.+four.+two.+three',
  'priority-,description+'          => 'four.+one.+two.+three',
  'priority+,description-'          => 'three.+two.+one.+four',
  'priority+,description+'          => 'three.+two.+four.+one',

  'project-,priority-'              => 'four.+three.+two.+one',
  'project-,priority+'              => 'three.+four.+two.+one',
  'project+,priority-'              => 'one.+two.+four.+three',
  'project+,priority+'              => 'one.+two.+three.+four',

  'project-,active-'                => 'three.+four.+two.+one',
  'project-,active+'                => 'four.+three.+two.+one',
  'project+,active-'                => 'one.+two.+three.+four',
  'project+,active+'                => 'one.+two.+four.+three',

  'project-,due-'                   => 'three.+four.+two.+one',
  'project-,due+'                   => 'four.+three.+two.+one',
  'project+,due-'                   => 'one.+two.+three.+four',
  'project+,due+'                   => 'one.+two.+four.+three',

  'project-,description-'           => 'three.+four.+two.+one',
  'project-,description+'           => 'four.+three.+two.+one',
  'project+,description-'           => 'one.+two.+three.+four',
  'project+,description+'           => 'one.+two.+four.+three',

  'active-,priority-'               => 'one.+three.+four.+two',
  'active-,priority+'               => 'three.+one.+two.+four',
  'active+,priority-'               => 'four.+two.+one.+three',
  'active+,priority+'               => 'two.+four.+three.+one',

  'active-,project-'                => 'three.+one.+four.+two',
  'active-,project+'                => 'one.+three.+two.+four',
  'active+,project-'                => 'four.+two.+three.+one',
  'active+,project+'                => 'two.+four.+one.+three',

  'active-,due-'                    => 'three.+one.+(?:four.+two|two.+four)',
  'active-,due+'                    => 'one.+three.+(?:four.+two|two.+four)',
  'active+,due-'                    => '(?:four.+two|two.+four).+three.+one',
  'active+,due+'                    => '(?:four.+two|two.+four).+one.+three',

  'active-,description-'            => 'three.+one.+two.+four',
  'active-,description+'            => 'one.+three.+four.+two',
  'active+,description-'            => 'two.+four.+three.+one',
  'active+,description+'            => 'four.+two.+one.+three',

  'due-,priority-'                  => 'three.+four.+two.+one',
  'due-,priority+'                  => 'three.+two.+four.+one',
  'due+,priority-'                  => 'one.+four.+two.+three',
  'due+,priority+'                  => 'one.+two.+four.+three',

  'due-,project-'                   => 'three.+four.+two.+one',
  'due-,project+'                   => 'three.+two.+four.+one',
  'due+,project-'                   => 'one.+four.+two.+three',
  'due+,project+'                   => 'one.+two.+four.+three',

  'due-,active-'                    => 'three.+(?:four.+two|two.+four).+one',
  'due-,active+'                    => 'three.+(?:four.+two|two.+four).+one',
  'due+,active-'                    => 'one.+(?:four.+two|two.+four).+three',
  'due+,active+'                    => 'one.+(?:four.+two|two.+four).+three',

  'due-,description-'               => 'three.+two.+four.+one',
  'due-,description+'               => 'three.+four.+two.+one',
  'due+,description-'               => 'one.+two.+four.+three',
  'due+,description+'               => 'one.+four.+two.+three',

  'description-,priority-'          => 'two.+three.+one.+four',
  'description-,priority+'          => 'two.+three.+one.+four',
  'description+,priority-'          => 'four.+one.+three.+two',
  'description+,priority+'          => 'four.+one.+three.+two',

  'description-,project-'           => 'two.+three.+one.+four',
  'description-,project+'           => 'two.+three.+one.+four',
  'description+,project-'           => 'four.+one.+three.+two',
  'description+,project+'           => 'four.+one.+three.+two',

  'description-,active-'            => 'two.+three.+one.+four',
  'description-,active+'            => 'two.+three.+one.+four',
  'description+,active-'            => 'four.+one.+three.+two',
  'description+,active+'            => 'four.+one.+three.+two',

  'description-,due-'               => 'two.+three.+one.+four',
  'description-,due+'               => 'two.+three.+one.+four',
  'description+,due-'               => 'four.+one.+three.+two',
  'description+,due+'               => 'four.+one.+three.+two',

  # Four sort columns.
  'active+,project+,due+,priority+' => 'two.+four.+one.+three',
  'project+,due+,priority+,active+' => 'one.+two.+four.+three',
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

