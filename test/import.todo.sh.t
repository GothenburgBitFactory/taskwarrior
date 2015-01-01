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
use Test::More tests => 23;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

my $source_dir = $0;
$source_dir =~ s{[^/]+$}{..};

# Create the rc file.
if (open my $fh, '>', 'import.rc')
{
  print $fh "data.location=.\n",
            "json.array=on\n",
            "dateformat=YYYY-M-D\n",
            "verbose=off\n";
  close $fh;
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
}

# Convert todo.sh --> task JSON.
qx{$source_dir/scripts/add-ons/import-todo.sh.pl <import.txt >import.json 2>&1};

# Import the JSON.
my $output = qx{../src/task rc:import.rc import import.json 2>&1};
diag ($output);

$output = qx{../src/task rc:import.rc info 1 2>&1};
like ($output, qr/^Priority.+H/ms, '1 pri:H');
like ($output, qr/^Tags.+phone/ms, '1 +phone');
like ($output, qr/^Description.+\@phone thank Mom for the meatballs/ms, '1 <desc>');

$output = qx{../src/task rc:import.rc info 2 2>&1};
like ($output, qr/^Priority.+M/ms, '2 pri:M');
like ($output, qr/^Project.+GarageSale/ms, '2 <project>');
like ($output, qr/^Description.+/ms, '2 <desc>');

$output = qx{../src/task rc:import.rc info 3 2>&1};
like ($output, qr/^Project.+GarageSale/ms, '3 <project>');
like ($output, qr/^Description.+\+GarageSale \@home post signs around the neighborhood/ms, '3 <desc>');

$output = qx{../src/task rc:import.rc info 4 2>&1};
like ($output, qr/^Description.+\@shopping Eskimo pies/ms, '4 <desc>');

$output = qx{../src/task rc:import.rc info 5 2>&1};
like ($output, qr/^Priority.+H/ms, '5 pri:H');
like ($output, qr/^Description.+Call Mom/ms, '5 <desc>');

$output = qx{../src/task rc:import.rc info 6 2>&1};
like ($output, qr/^Description.+Really gotta call Mom \(A\) \@phone \@someday/ms, '6 <desc>');

$output = qx{../src/task rc:import.rc info 7 2>&1};
like ($output, qr/^Description.+\(b\)->get back to the boss/ms, '7 <desc>');

$output = qx{../src/task rc:import.rc info 8 2>&1};
like ($output, qr/^Project.+TodoTxt/ms, '8 <project>');
like ($output, qr/^Description.+Document \+TodoTxt task format/ms, '8 <desc>');

$output = qx{../src/task rc:import.rc info 9 2>&1};
like ($output, qr/^Priority.+H/ms, '9 pri:H');
like ($output, qr/^Description.+Call Mom/ms, '9 <desc>');

$output = qx{../src/task rc:import.rc info 10 2>&1};
like ($output, qr/^Priority.+H/ms, '10 pri:H');
like ($output, qr/^Description.+Call Mom 2011-03-02/ms, '10 <desc>');

$output = qx{../src/task rc:import.rc info 11 2>&1};
like ($output, qr/^Priority.+H/ms, '11 pri:H');
like ($output, qr/^Project.+Family/ms, '8 <project>');
like ($output, qr/^Description.+Call Mom \+Family \+PeaceLoveAndHappiness \@iphone \@phone/ms, '11 <desc>');

$output = qx{../src/task rc:import.rc info 12 2>&1};
like ($output, qr/^Description.+xylophone lesson/ms, '12 <desc>');

# TODO and now the completed ones.

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data import.rc import.txt import.json);
exit 0;

