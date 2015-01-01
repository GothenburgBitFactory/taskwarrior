#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

use strict;
use warnings;
use Test::More tests => 98;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'sorting.rc')
{
  print $fh "data.location=.\n";
  close $fh;
}

# Test assorted sort orders.
qx{../src/task rc:sorting.rc add                                    zero 2>&1};
qx{../src/task rc:sorting.rc add priority:H project:A due:yesterday one 2>&1};
qx{../src/task rc:sorting.rc add priority:M project:B due:today     two 2>&1};
qx{../src/task rc:sorting.rc add priority:L project:C due:tomorrow  three 2>&1};
qx{../src/task rc:sorting.rc add priority:H project:C due:today     four 2>&1};
qx{../src/task rc:sorting.rc 2 start 2>&1};
#diag (qx{../src/task rc:sorting.rc list});

my %tests =
(
  # Single sort column.
  'priority-'                       => ['(?:one.+four|four.+one).+two.+three.+zero'],
  'priority+'                       => ['zero.+three.+two.+(?:one.+four|four.+one)'],
  'project-'                        => ['(?:three.+four|four.+three).+two.+one.+zero'],
  'project+'                        => ['zero.+one.+two.+(?:three.+four|four.+three)'],
  'start-'                          => ['one.+zero', 'one.+two', 'one.+three', 'one.+four'],
  'start+'                          => ['zero.+one', 'two.+one', 'three.+one', 'four.+one'],
  'due-'                            => ['three.+(?:two.+four|four.+two).+one.+zero'],
  'due+'                            => ['one.+(?:two.+four|four.+two).+three.+zero'],
  'description-'                    => ['zero.+two.+three.+one.+four'],
  'description+'                    => ['four.+one.+three.+two.+zero'],

  # Two sort columns.
  'priority-,project-'              => ['four.+one.+two.+three.+zero'],
  'priority-,project+'              => ['one.+four.+two.+three.+zero'],
  'priority+,project-'              => ['zero.+three.+two.+four.+one'],
  'priority+,project+'              => ['zero.+three.+two.+one.+four'],

  'priority-,start-'                => ['one.+four.+two.+three.+zero'],
  'priority-,start+'                => ['four.+one.+two.+three.+zero'],
  'priority+,start-'                => ['zero.+three.+two.+one.+four'],
  'priority+,start+'                => ['zero.+three.+two.+four.+one'],

  'priority-,due-'                  => ['four.+one.+two.+three.+zero'],
  'priority-,due+'                  => ['one.+four.+two.+three.+zero'],
  'priority+,due-'                  => ['zero.+three.+two.+four.+one'],
  'priority+,due+'                  => ['zero.+three.+two.+one.+four'],

  'priority-,description-'          => ['one.+four.+two.+three.+zero'],
  'priority-,description+'          => ['four.+one.+two.+three.+zero'],
  'priority+,description-'          => ['zero.+three.+two.+one.+four'],
  'priority+,description+'          => ['zero.+three.+two.+four.+one'],

  'project-,priority-'              => ['four.+three.+two.+one.+zero'],
  'project-,priority+'              => ['three.+four.+two.+one.+zero'],
  'project+,priority-'              => ['zero.+one.+two.+four.+three'],
  'project+,priority+'              => ['zero.+one.+two.+three.+four'],

  'project-,start-'                 => ['three.+four.+two.+one.+zero'],
  'project-,start+'                 => ['(?:four.+three|three.+four).+two.+one.+zero'],
  'project+,start-'                 => ['zero.+one.+two.+three.+four'],
  'project+,start+'                 => ['zero.+one.+two.+(?:four.+three|three.+four)'],

  'project-,due-'                   => ['three.+four.+two.+one.+zero'],
  'project-,due+'                   => ['four.+three.+two.+one.+zero'],
  'project+,due-'                   => ['zero.+one.+two.+three.+four'],
  'project+,due+'                   => ['zero.+one.+two.+four.+three'],

  'project-,description-'           => ['three.+four.+two.+one.+zero'],
  'project-,description+'           => ['four.+three.+two.+one.+zero'],
  'project+,description-'           => ['zero.+one.+two.+three.+four'],
  'project+,description+'           => ['zero.+one.+two.+four.+three'],

  'start-,priority-'                => ['one.+four.+two.+three.+zero'],
  'start-,priority+'                => ['one.+zero.+three.+two.+four'],
  'start+,priority-'                => ['four.+two.+three.+zero.+one'],
  'start+,priority+'                => ['zero.+three.+two.+four.+one'],

  'start-,project-'                 => ['one.+(?:three.+four|four.+three).+two.+zero'],
  'start-,project+'                 => ['one.+zero.+two.+(?:three.+four|four.+three)'],
  'start+,project-'                 => ['(?:three.+four|four.+three).+two.+zero.+one'],
  'start+,project+'                 => ['zero.+two.+(?:three.+four|four.+three).+one'],

  'start-,due-'                     => ['one.+three.+(?:four.+two|two.+four).+zero'],
  'start-,due+'                     => ['one.+(?:four.+two|two.+four).+three.+zero'],
  'start+,due-'                     => ['three.+(?:four.+two|two.+four).+zero.+one'],
  'start+,due+'                     => ['(?:four.+two|two.+four).+three.+zero.+one'],

  'start-,description-'             => ['one.+zero.+two.+three.+four'],
  'start-,description+'             => ['one.+four.+three.+two.+zero'],
  'start+,description-'             => ['zero.+two.+three.+four.+one'],
  'start+,description+'             => ['four.+three.+two.+zero.+one'],

  'due-,priority-'                  => ['three.+four.+two.+one.+zero'],
  'due-,priority+'                  => ['three.+two.+four.+one.+zero'],
  'due+,priority-'                  => ['one.+four.+two.+three.+zero'],
  'due+,priority+'                  => ['one.+two.+four.+three.+zero'],

  'due-,project-'                   => ['three.+four.+two.+one.+zero'],
  'due-,project+'                   => ['three.+two.+four.+one.+zero'],
  'due+,project-'                   => ['one.+four.+two.+three.+zero'],
  'due+,project+'                   => ['one.+two.+four.+three.+zero'],

  'due-,start-'                     => ['three.+(?:four.+two|two.+four).+one.+zero'],
  'due-,start+'                     => ['three.+(?:four.+two|two.+four).+one.+zero'],
  'due+,start-'                     => ['one.+(?:four.+two|two.+four).+three.+zero'],
  'due+,start+'                     => ['one.+(?:four.+two|two.+four).+three.+zero'],

  'due-,description-'               => ['three.+two.+four.+one.+zero'],
  'due-,description+'               => ['three.+four.+two.+one.+zero'],
  'due+,description-'               => ['one.+two.+four.+three.+zero'],
  'due+,description+'               => ['one.+four.+two.+three.+zero'],

  'description-,priority-'          => ['zero.+two.+three.+one.+four'],
  'description-,priority+'          => ['zero.+two.+three.+one.+four'],
  'description+,priority-'          => ['four.+one.+three.+two.+zero'],
  'description+,priority+'          => ['four.+one.+three.+two.+zero'],

  'description-,project-'           => ['zero.+two.+three.+one.+four'],
  'description-,project+'           => ['zero.+two.+three.+one.+four'],
  'description+,project-'           => ['four.+one.+three.+two.+zero'],
  'description+,project+'           => ['four.+one.+three.+two.+zero'],

  'description-,start-'             => ['zero.+two.+three.+one.+four'],
  'description-,start+'             => ['zero.+two.+three.+one.+four'],
  'description+,start-'             => ['four.+one.+three.+two.+zero'],
  'description+,start+'             => ['four.+one.+three.+two.+zero'],

  'description-,due-'               => ['zero.+two.+three.+one.+four'],
  'description-,due+'               => ['zero.+two.+three.+one.+four'],
  'description+,due-'               => ['four.+one.+three.+two.+zero'],
  'description+,due+'               => ['four.+one.+three.+two.+zero'],

  # Four sort columns.
  'start+,project+,due+,priority+'  => ['zero.+two.+four.+three.+one'],
  'project+,due+,priority+,start+'  => ['zero.+one.+two.+four.+three'],
);

for my $sort (sort keys %tests)
{
  my $output = qx{../src/task rc:sorting.rc rc.report.list.sort:${sort} list 2>&1};
  for my $expectation (@{$tests{$sort}})
  {
    like ($output, qr/$expectation/ms, "sort:${sort}");
  }
}

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data sorting.rc);
exit 0;

