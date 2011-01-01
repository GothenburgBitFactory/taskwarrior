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
use Test::More tests => 23;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/Y\n",
            "report.foo.description=Sample\n",
            "report.foo.columns=id,due,description\n",
            "report.foo.labels=ID,Due,Description\n",
            "report.foo.sort=due+\n",
            "report.foo.filter=status:pending\n",
            "report.foo.dateformat=MD\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug #418: due.before:eow not working
#   - with dateformat is MD
qx{../src/task rc:bug.rc add one   due:6/28/2010};
qx{../src/task rc:bug.rc add two   due:6/29/2010};
qx{../src/task rc:bug.rc add three due:6/30/2010};
qx{../src/task rc:bug.rc add four  due:7/1/2010};
qx{../src/task rc:bug.rc add five  due:7/2/2010};
qx{../src/task rc:bug.rc add six   due:7/3/2010};
qx{../src/task rc:bug.rc add seven due:7/4/2010};
qx{../src/task rc:bug.rc add eight due:7/5/2010};
qx{../src/task rc:bug.rc add nine  due:7/6/2010};

my $output = qx{../src/task rc:bug.rc foo};
like ($output, qr/one/ms,     'task 1 listed');
like ($output, qr/two/ms,     'task 2 listed');
like ($output, qr/three/ms,   'task 3 listed');
like ($output, qr/four/ms,    'task 4 listed');
like ($output, qr/five/ms,    'task 5 listed');
like ($output, qr/six/ms,     'task 6 listed');
like ($output, qr/seven/ms,   'task 7 listed');
like ($output, qr/eight/ms,   'task 8 listed');
like ($output, qr/nine/ms,    'task 9 listed');

$output = qx{../src/task rc:bug.rc foo due.before:7/2/2010};
like ($output, qr/one/ms,     'task 1 listed');
like ($output, qr/two/ms,     'task 2 listed');
like ($output, qr/three/ms,   'task 3 listed');
like ($output, qr/four/ms,    'task 4 listed');
unlike ($output, qr/five/ms,  'task 5 not listed');
unlike ($output, qr/six/ms,   'task 6 not listed');
unlike ($output, qr/seven/ms, 'task 7 not listed');
unlike ($output, qr/eight/ms, 'task 8 not listed');
unlike ($output, qr/nine/ms,  'task 9 not listed');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'bug.rc';
ok (!-r 'bug.rc', 'Removed bug.rc');

exit 0;

