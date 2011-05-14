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
use Test::More tests => 30;

# Create the rc file.
if (open my $fh, '>', 'import.rc')
{
  print $fh "data.location=.\n",
            "dateformat=YYYY-M-D\n";
  close $fh;
  ok (-r 'import.rc', 'Created import.rc');
}

# Create import file.
if (open my $fh, '>', 'import.txt')
{
  print $fh "(A) Prioritized todo\n",
            "(B) 2011-05-09 Prioritized with entry date\n",
            "2011-05-09 Not prioritized with entry date\n",
            "(C) +Project \@Context Priority with Project and context t:2011-05-11 < due date\n",
            "(D) 2011-05-09 +Project +OtherProject \@Context \@OtherContext Two Projects and Context with date\n",
            "x 2011-05-11 2011-05-09 +Project +OtherProject \@Context with date and completion date\n";
  close $fh;
  ok (-r 'import.txt', 'Created sample import data');
}

my $output = qx{../src/task rc:import.rc import import.txt};
like ($output, qr/Imported 6 tasks successfully, with 0 errors./, 'no errors');

$output = qx{../src/task rc:import.rc info 1};
like ($output, qr/^Priority.+H.*$/m,                                     'priority (A) -> pri:H');
like ($output, qr/^Description.+Prioritized todo.*$/m,                   '... -> desc:...');

$output = qx{../src/task rc:import.rc info 2};
like ($output, qr/^Priority.+M.*$/m,                                     'priority (B) -> pri:M');
like ($output, qr/^Description.+Prioritized with entry Date.*$/m,        '... -> desc:...');

$output = qx{../src/task rc:import.rc info 3};
like ($output, qr/^Project.+Project.*$/m,                                '+Project -> pro:Project');
like ($output, qr/^Description.+Not prioritized with entry Date.*$/m,    '... -> desc:...');

$output = qx{../src/task rc:import.rc info 4};
like ($output, qr/^Priority.+L.*$/m,                                     'priority (C) -> pri:L');
like ($output, qr/^Project.+Project.*$/m,                                '+Project -> pro:Project');
like ($output, qr/^Description.+Priority with Project and context.*$/m,  '... -> desc:...');
like ($output, qr/^Tags.+\@Context.*$/m,                                 '@Context -> +@Context');
like ($output, qr/^Due.+2011-05-11.*$/m,                                 't:2011-05-11 -> due:2011-05-11');

$output = qx{../src/task rc:import.rc info 5};
unlike ($output, qr/^Priority/m,                                         'priority (D) -> pri:');
like ($output, qr/^Project.+Project.*$/m,                                '+Project -> pro:Project');
like ($output, qr/^Description.+Two Projects and Context with date.*$/m, '... -> desc:...');
like ($output, qr/^Tags.+\@Context.*$/m,                                 '@Context -> +@Context');
like ($output, qr/^Entry.+2011-05-09.*$/m,                               '2011-05-09 -> entry:2011-05-09');

$output = qx{../src/task rc:import.rc info 6};
like ($output, qr/^Description.+With date and completion date.*$/m,      '... -> desc:...');
like ($output, qr/^Description.+With date and completion date.*$/m,      '... -> desc:...');
like ($output, qr/^Tags.+\@Context.*$/m,                                 '@Context -> +@Context');
like ($output, qr/^Entry.+2011-05-09.*$/m,                               '2011-05-09 -> entry:2011-05-09');
like ($output, qr/^End.+2011-05-11.*$/m,                                 '2011-05-11 -> end:2011-05-11');
like ($output, qr/^Status.+Completed.*$/m,                               'x -> status:Completed');

# Cleanup.
unlink 'import.txt';
ok (!-r 'import.txt', 'Removed import.txt');

unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'import.rc';
ok (!-r 'import.rc', 'Removed import.rc');

exit 0;

