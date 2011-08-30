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
if (open my $fh, '>', 'import.rc')
{
  print $fh "data.location=.\n",
            "json.array=on\n",
            "dateformat=YYYY-M-D\n";
  close $fh;
  ok (-r 'import.rc', 'Created import.rc');
}

# Create import file.
if (open my $fh, '>', 'import.txt')
{
  print $fh "(A) \@phone thank Mom for the meatballs\n",
            "(B) +GarageSale \@phone schedule Goodwill pickup\n",
            "+GarageSale \@home post signs around the neighborhood\n",
            "\@shopping Eskimo pies\n",
            "(A) Call Mom\n",
            "Really gotta call Mom (A) \@phone \@someday\n",
            "(b)->get back to the boss\n",
            "2011-03-02 Document +TodoTxt task format\n",
            "(A) 2011-03-02 Call Mom\n",
            "(A) Call Mom 2011-03-02\n",
            "(A) Call Mom +Family +PeaceLoveAndHappiness \@iphone \@phone\n",
            "X 2011-03-03 Call Mom\n",
            "xylophone lesson\n",
            "x 2011-03-02 2011-03-01 Review Tim's pull request +TodoTxtTouch \@github\n";

  close $fh;
  ok (-r 'import.txt', 'Created sample import data');
}

# Convert todo.sh --> task JSON.
my $output = qx{../scripts/add-ons/import-todo.sh.pl <import.txt >json.txt};
diag ($output);

# Import the JSON.
$output = qx{../src/task rc:import.rc import json.txt};
diag ($output);

$output = qx{../src/task rc:import.rc info 1};
diag ($output);
like ($output, qr/^Priority.+H/ms, '1 pri:H');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key import.rc import.txt json.txt);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch_key.data' &&
    ! -r 'import.rc'      &&
    ! -r 'import.txt'     &&
    ! -r 'json.txt', 'Cleanup');

exit 0;

