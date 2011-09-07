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
use Test::More tests => 26;

# Create the rc file.
if (open my $fh, '>', 'import.rc')
{
  print $fh "data.location=.\n",
            "json.array=on\n",
            "dateformat=YYYY-M-D\n",
            "verbose=off\n";
  close $fh;
  ok (-r 'import.rc', 'Created import.rc');
}

# Create import file.
if (open my $fh, '>', 'import.txt')
{
  print $fh "(A) \@phone thank Mom for the meatballs\n",                                   # 1
            "(B) +GarageSale \@phone schedule Goodwill pickup\n",                          # 2
            "+GarageSale \@home post signs around the neighborhood\n",                     # 3
            "\@shopping Eskimo pies\n",                                                    # 4
            "(A) Call Mom\n",                                                              # 5
            "Really gotta call Mom (A) \@phone \@someday\n",                               # 6
            "(b)->get back to the boss\n",                                                 # 7
            "2011-03-02 Document +TodoTxt task format\n",                                  # 8
            "(A) 2011-03-02 Call Mom\n",                                                   # 9
            "(A) Call Mom 2011-03-02\n",                                                   # 10
            "(A) Call Mom +Family +PeaceLoveAndHappiness \@iphone \@phone\n",              # 11
            "xylophone lesson\n",                                                          # 12
            "X 2011-03-03 Call Mom\n",                                                     # -
            "x 2011-03-02 2011-03-01 Review Tim's pull request +TodoTxtTouch \@github\n";  # -

  close $fh;
  ok (-r 'import.txt', 'Created sample import data');
}

# Convert todo.sh --> task JSON.
qx{../scripts/add-ons/import-todo.sh.pl <import.txt >import.json};

# Import the JSON.
my $output = qx{../src/task rc:import.rc import import.json};
diag ($output);

$output = qx{../src/task rc:import.rc info 1};
like ($output, qr/^Priority.+H/ms, '1 pri:H');
like ($output, qr/^Tags.+phone/ms, '1 +phone');
like ($output, qr/^Description.+\@phone thank Mom for the meatballs/ms, '1 <desc>');

$output = qx{../src/task rc:import.rc info 2};
like ($output, qr/^Priority.+M/ms, '2 pri:M');
like ($output, qr/^Project.+GarageSale/ms, '2 <project>');
like ($output, qr/^Description.+/ms, '2 <desc>');

$output = qx{../src/task rc:import.rc info 3};
like ($output, qr/^Project.+GarageSale/ms, '3 <project>');
like ($output, qr/^Description.+\+GarageSale \@home post signs around the neighborhood/ms, '3 <desc>');

$output = qx{../src/task rc:import.rc info 4};
like ($output, qr/^Description.+\@shopping Eskimo pies/ms, '4 <desc>');

$output = qx{../src/task rc:import.rc info 5};
like ($output, qr/^Priority.+H/ms, '5 pri:H');
like ($output, qr/^Description.+Call Mom/ms, '5 <desc>');

$output = qx{../src/task rc:import.rc info 6};
like ($output, qr/^Description.+Really gotta call Mom \(A\) \@phone \@someday/ms, '6 <desc>');

$output = qx{../src/task rc:import.rc info 7};
like ($output, qr/^Description.+\(b\)->get back to the boss/ms, '7 <desc>');

$output = qx{../src/task rc:import.rc info 8};
like ($output, qr/^Project.+TodoTxt/ms, '8 <project>');
like ($output, qr/^Description.+Document \+TodoTxt task format/ms, '8 <desc>');

$output = qx{../src/task rc:import.rc info 9};
like ($output, qr/^Priority.+H/ms, '9 pri:H');
like ($output, qr/^Description.+Call Mom/ms, '9 <desc>');

$output = qx{../src/task rc:import.rc info 10};
like ($output, qr/^Priority.+H/ms, '10 pri:H');
like ($output, qr/^Description.+Call Mom 2011-03-02/ms, '10 <desc>');

$output = qx{../src/task rc:import.rc info 11};
like ($output, qr/^Priority.+H/ms, '11 pri:H');
like ($output, qr/^Project.+Family/ms, '8 <project>');
like ($output, qr/^Description.+Call Mom \+Family \+PeaceLoveAndHappiness \@iphone \@phone/ms, '11 <desc>');

$output = qx{../src/task rc:import.rc info 12};
like ($output, qr/^Description.+xylophone lesson/ms, '12 <desc>');

# TODO and now the completed ones.

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key import.rc import.txt import.json);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'import.rc'      &&
    ! -r 'import.txt'     &&
    ! -r 'import.json', 'Cleanup');

exit 0;

