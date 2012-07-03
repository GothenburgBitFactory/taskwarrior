#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 31;
use File::Copy;
use File::Path;

use constant false => 0;
use constant true => 1;

# Create data locations
mkdir("data1",  0755);
mkdir("data2",  0755);
mkdir("data3",  0755);
mkdir('backup', 0755);
ok(-e 'data1', "Created directory data1");
ok(-e 'data2', "Created directory data2");
ok(-e 'data3', "Created directory data3");
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
qx{../src/task rc:1.rc add Task1 2>&1};
diag ("7 second delay");
sleep(1);
qx{../src/task rc:1.rc add Task2 2>&1};
sleep(1);
qx{../src/task rc:1.rc add Task3 2>&1};
sleep(1);
qx{../src/task rc:1.rc add Task4 2>&1};

# Merge with backup
my $output = qx{../src/task rc:1.rc push ./backup/ 2>&1};

#######################################
# Modify on 2nd resource

# first merge
$output = qx{../src/task rc:2.rc merge 2>&1};
like ($output, qr/Merge complete/, "res2: pre-merge completed");

# complete Task1
qx{../src/task rc:2.rc 1 done 2>&1};
sleep(1);

#######################################
# Modify on 3rd resource

# first merge
$output = qx{../src/task rc:3.rc merge 2>&1};
like ($output, qr/Merge complete/, "res3: pre-merge completed");

# complete Task1
qx{../src/task rc:3.rc 1 done 2>&1};
sleep(1);

# now merge 3rd resource
$output = qx{../src/task rc:3.rc merge 2>&1};
like ($output, qr/Merge complete/, "res3: post-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

# and merge 2nd resource
$output = qx{../src/task rc:2.rc merge 2>&1};
like ($output, qr/Merge complete/, "res2: post-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

# merge 3rd
$output = qx{../src/task rc:3.rc merge 2>&1};
like ($output, qr/Merge complete/, "res3: post-merge completed");
unlike ($output, qr/Missing/, "no missing entry");
like ($output, qr/Retain/,  "retained changes");  # 16

# pre-merge 1st
$output = qx{../src/task rc:1.rc merge 2>&1};
like ($output, qr/Merge complete/, "res1: pre-merge completed");   # 17
unlike ($output, qr/Missing/, "no missing entry");

qx{../src/task rc:1.rc add Task5 2>&1};
sleep(1);
qx{../src/task rc:1.rc 4 done 2>&1};
sleep(1);

# merge
$output = qx{../src/task rc:1.rc merge 2>&1};
like ($output, qr/Merge complete/, "res1: post-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

# pre-merge 2nd res
$output = qx{../src/task rc:2.rc merge 2>&1};
like ($output, qr/Merge complete/, "res2: pre-merge completed");
unlike ($output, qr/Missing/, "no missing entry");

# merge
$output = qx{../src/task rc:1.rc merge 2>&1};
like ($output, qr/up-to-date/, "res1: up-to-date");
unlike ($output, qr/Missing/, "no missing entry");

# pre-merge 2nd res
$output = qx{../src/task rc:2.rc merge 2>&1};
like ($output, qr/up-to-date/, "res2: up-to-date");
unlike ($output, qr/Missing/, "no missing entry");

# Cleanup.
unlink qw(data1/pending.data data1/completed.data data1/undo.data data1/undo.save data1/backlog.data data1/synch.key 1.rc);
ok (! -r 'data1/pending.data'   &&
    ! -r 'data1/completed.data' &&
    ! -r 'data1/undo.data'      &&
    ! -r 'data1/undo.save'      &&
    ! -r 'data1/backlog.data'   &&
    ! -r 'data1/synch.key'      &&
    ! -r '1.rc', 'data1 Cleanup');

unlink qw(data2/pending.data data2/completed.data data2/undo.data data2/undo.save data2/backlog.data data2/synch.key 2.rc);
ok (! -r 'data2/pending.data'   &&
    ! -r 'data2/completed.data' &&
    ! -r 'data2/undo.data'      &&
    ! -r 'data2/undo.save'      &&
    ! -r 'data2/backlog.data'   &&
    ! -r 'data2/synch.key'      &&
    ! -r '2.rc', 'data2 Cleanup');

unlink qw(data3/pending.data data3/completed.data data3/undo.data data3/undo.save data3/backlog.data data3/synch.key 3.rc);
ok (! -r 'data3/pending.data'   &&
    ! -r 'data3/completed.data' &&
    ! -r 'data3/undo.data'      &&
    ! -r 'data3/undo.save'      &&
    ! -r 'data3/backlog.data'   &&
    ! -r 'data3/synch.key'      &&
    ! -r '3.rc', 'data3 Cleanup');

unlink qw(backup/pending.data backup/completed.data backup/undo.data backup/undo.save backup/backlog.data backup/synch.key);
ok (! -r 'backup/pending.data'   &&
    ! -r 'backup/completed.data' &&
    ! -r 'backup/undo.data'      &&
    ! -r 'backup/undo.save'      &&
    ! -r 'backup/backlog.data'   &&
    ! -r 'backup/synch.key', 'backup Cleanup');

rmtree (['data1/extensions', 'data1', 'data2/extensions', 'data2', 'data3/extensions', 'data3', 'backup/extensions', 'backup'], 0, 1);
ok (! -e 'data1/extensions'  &&
    ! -e 'data1'             &&
    ! -e 'data2/extensions'  &&
    ! -e 'data2'             &&
    ! -e 'data3/extensions'  &&
    ! -e 'data3'             &&
    ! -e 'backup/extensions' &&
    ! -e 'backup', 'Removed dir local');

exit 0;

