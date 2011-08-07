#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Johannes Schlatow.
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
use Test::More tests => 45;
use File::Copy;

use constant false => 0;
use constant true => 1;

# Create data locations
mkdir("data1", 0755);
ok(-e 'data1', "Created directory data1");
mkdir("data2", 0755);
ok(-e 'data2', "Created directory data2");
mkdir("data3", 0755);
ok(-e 'data3', "Created directory data3");
mkdir('backup', 0755);
ok(-e 'backup', "Created directory backup");

# Create the rc files.
if (open my $fh, '>', '1.rc')
{
  print $fh "data.location=./data1\n",
            "confirmation=no\n",
				"merge.autopush=yes\n",
				"merge.default.uri=./backup/\n",
            "report.list.description=DESC\n",
				"report.list.columns=id,project,active,priority,description,tags\n",
				"report.list.labels=id,pro,a,pri,d,t\n",
				"report.list.sort=id+\n",
				"report.list.filter=status:pending\n";
  close $fh;
  ok (-r '1.rc', 'Created 1.rc');
}

# Create the rc files.
if (open my $fh, '>', '2.rc')
{
  print $fh "data.location=./data2\n",
            "confirmation=no\n",
				"merge.autopush=yes\n",
				"merge.default.uri=./backup/\n",
            "report.list.description=DESC\n",
				"report.list.columns=id,project,active,priority,description,tags\n",
				"report.list.labels=id,pro,a,pri,d,t\n",
				"report.list.sort=id+\n",
				"report.list.filter=status:pending\n";
  close $fh;
  ok (-r '2.rc', 'Created 2.rc');
}

# Create the rc files.
if (open my $fh, '>', '3.rc')
{
  print $fh "data.location=./data3\n",
            "confirmation=no\n",
				"merge.autopush=yes\n",
				"merge.default.uri=./backup/\n",
            "report.list.description=DESC\n",
				"report.list.columns=id,project,active,priority,description,tags\n",
				"report.list.labels=id,pro,a,pri,d,t\n",
				"report.list.sort=id+\n",
				"report.list.filter=status:pending\n";
  close $fh;
  ok (-r '3.rc', 'Created 3.rc');
}

#######################################
# Create tasks on 1st resource
qx{../src/task rc:1.rc add Task1};
diag ("7 second delay");
sleep(1);
qx{../src/task rc:1.rc add Task2};
sleep(1);
qx{../src/task rc:1.rc add Task3};
sleep(1);
qx{../src/task rc:1.rc add Task4};

# Merge with backup
my $output = qx{../src/task rc:1.rc push ./backup/};

#######################################
# Modify on 2nd resource

# first merge
$output = qx{../src/task rc:2.rc merge};
like ($output, qr/Merge complete/, "res2: pre-merge completed");

# complete Task1
qx{../src/task rc:2.rc 1 done};
sleep(1);

#######################################
# Modify on 3rd resource

# first merge
$output = qx{../src/task rc:3.rc merge};
like ($output, qr/Merge complete/, "res3: pre-merge completed");

# complete Task1
qx{../src/task rc:3.rc 1 done};
sleep(1);

# now merge 3rd resource
$output = qx{../src/task rc:3.rc merge};
like ($output, qr/Merge complete/, "res3: post-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

# and merge 2nd resource
$output = qx{../src/task rc:2.rc merge};
like ($output, qr/Merge complete/, "res2: post-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

# merge 3rd
$output = qx{../src/task rc:3.rc merge};
like ($output, qr/Merge complete/, "res3: post-merge completed");
unlike ($output, qr/Missing/, "no missing entry");
like ($output, qr/Retain/,  "retained changes");

# pre-merge 1st
$output = qx{../src/task rc:1.rc merge};
like ($output, qr/Merge complete/, "res1: pre-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

qx{../src/task rc:1.rc add Task5};
sleep(1);
qx(../src/task rc:1.rc 4 done);
sleep(1);

# merge
$output = qx{../src/task rc:1.rc merge};
like ($output, qr/Merge complete/, "res1: post-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

# pre-merge 2nd res
$output = qx{../src/task rc:2.rc merge};
like ($output, qr/Merge complete/, "res2: pre-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

# merge
$output = qx{../src/task rc:1.rc merge};
like ($output, qr/up-to-date/, "res1: up-to-date");
unlike ($output, qr/Missing/, "no missing entry");

# pre-merge 2nd res
$output = qx{../src/task rc:2.rc merge};
like ($output, qr/up-to-date/, "res2: up-to-date");
unlike ($output, qr/Missing/, "no missing entry");

# Cleanup.
unlink 'data1/pending.data';
ok (!-r 'data1/pending.data', 'Removed data1/pending.data');
unlink 'data1/completed.data';
ok (!-r 'data1/completed.data', 'Removed data1/completed.data');
unlink 'data1/undo.data';
ok (!-r 'data1/undo.data', 'Removed data1/undo.data');

unlink 'data2/pending.data';
ok (!-r 'data2/pending.data', 'Removed data2/pending.data');
unlink 'data2/completed.data';
ok (!-r 'data2/completed.data', 'Removed data2/completed.data');
unlink 'data2/undo.data';
ok (!-r 'data2/undo.data', 'Removed data2/undo.data');

unlink 'data3/pending.data';
ok (!-r 'data3/pending.data', 'Removed data3/pending.data');
unlink 'data3/completed.data';
ok (!-r 'data3/completed.data', 'Removed data3/completed.data');
unlink 'data3/undo.data';
ok (!-r 'data3/undo.data', 'Removed data3/undo.data');

unlink 'backup/pending.data';
ok (!-r 'backup/pending.data', 'Removed backup/pending.data');
unlink 'backup/completed.data';
ok (!-r 'backup/completed.data', 'Removed backup/completed.data');
unlink 'backup/undo.data';
ok (!-r 'backup/undo.data', 'Removed backup/undo.data');

unlink '1.rc';
ok (!-r '1.rc', 'Removed 1.rc');
unlink '2.rc';
ok (!-r '2.rc', 'Removed 2.rc');
unlink '3.rc';
ok (!-r '3.rc', 'Removed 3.rc');

rmdir("data1");
ok (!-e "data1", "Removed dir data1");
rmdir("data2");
ok (!-e "data2", "Removed dir data2");
rmdir("data3");
ok (!-e "data3", "Removed dir data3");
rmdir("backup");
ok (!-e "backup", "Removed dir backup");

exit 0;

