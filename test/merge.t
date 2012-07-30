#! /usr/bin/env perl
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
use Test::More tests => 33;
use File::Copy;
use File::Path;

use constant false => 0;
use constant true => 1;

# Create data locations
mkdir("local", 0755);
ok(-e 'local', "Created directory local");
mkdir("remote", 0755);
ok(-e 'remote', "Created directory remote");

# Create the rc files.
if (open my $fh, '>', 'local.rc')
{
  print $fh "data.location=./local\n",
            "confirmation=no\n",
            "merge.autopush=no\n",
            "report.list.description=DESC\n",
            "report.list.columns=id,project,start.active,priority,description,tags\n",
            "report.list.labels=id,pro,a,pri,d,t\n",
            "report.list.sort=id+\n",
            "report.list.filter=status:pending\n";
  close $fh;
  ok (-r 'local.rc', 'Created local.rc');
}

if (open my $fh, '>', 'remote.rc')
{
  print $fh "data.location=./remote\n",
            "confirmation=no\n",
            "merge.autopush=no\n",
            "report.list.description=DESC\n",
            "report.list.columns=id,project,start.active,priority,description,tags\n",
            "report.list.labels=id,pro,a,pri,d,t\n",
            "report.list.sort=id+\n",
            "report.list.filter=status:pending\n";
  close $fh;
  ok (-r 'remote.rc', 'Created remote.rc');
}

# Create some basic tasks on both sides
qx{../src/task rc:local.rc add left_modified 2>&1};
diag ("25 second delay");
sleep 1;
qx{../src/task rc:local.rc add right_modified 2>&1};
sleep 1;
qx{../src/task rc:local.rc add left_newer 2>&1};
sleep 1;
qx{../src/task rc:local.rc add right_newer 2>&1};
sleep 1;
qx{../src/task rc:local.rc add left_deleted 2>&1};
sleep 1;
qx{../src/task rc:local.rc add right_deleted 2>&1};
sleep 1;
qx{../src/task rc:local.rc add left_completed 2>&1};
sleep 1;
qx{../src/task rc:local.rc add right_completed 2>&1};
sleep 1;

copy ("local/undo.data",      "remote/undo.data");
copy ("local/pending.data",   "remote/pending.data");
copy ("local/completed.data", "remote/completed.data");

# make local modifications
qx{../src/task rc:local.rc add left_added 2>&1}; #left_added
sleep 1;
qx{../src/task rc:local.rc 1 modify prio:H 2>&1};       #left_modified
sleep 1;
qx{../src/task rc:local.rc 3 modify +stay 2>&1};        #left_newer
sleep 1;
qx{../src/task rc:local.rc 4 modify project:test 2>&1}; #right_newer
sleep 1;
qx{../src/task rc:local.rc 6 modify +delete 2>&1};      #right_deleted
sleep 1;

# make remote modifications
qx{../src/task rc:remote.rc add right_added 2>&1};    #right_added
sleep 1;
qx{../src/task rc:remote.rc 2 modify prio:L 2>&1};           #right_modified
sleep 1;
qx{../src/task rc:remote.rc 2 modify wait:tomorrow 2>&1};    #right_modified
sleep 1;
qx{../src/task rc:remote.rc 4 modify proj:realProject 2>&1}; #right_newer
sleep 1;
qx{../src/task rc:remote.rc 5 modify project:deletion 2>&1}; #left_deleted
sleep 1;
qx{../src/task rc:remote.rc 8 done 2>&1};             #right_completed
sleep 1;
qx{../src/task rc:remote.rc 6 del 2>&1};              #right_deleted
sleep 1;
qx{../src/task rc:remote.rc 3 done 2>&1};             #left_newer
sleep 1;

# make new local modifications
qx{../src/task rc:local.rc 3 start 2>&1};         #left_newer
sleep 1;
qx{../src/task rc:local.rc 4 modify +car 2>&1};   #right_newer
sleep 1;
qx{../src/task rc:local.rc 7 done 2>&1};          #left_completed
sleep 1;
qx{../src/task rc:local.rc 5 del 2>&1};           #left_deleted
sleep 1;

# make new remote modifications
qx{../src/task rc:remote.rc 4 modify +gym 2>&1};         # right_newer

# merge remote into local
copy ("local/undo.data", "local/undo.save");
my $output_l = qx{../src/task rc:local.rc merge remote/ 2>&1};

#check output
unlike ($output_l,   qr/Missing/,              "local-merge: no missing entry");
unlike ($output_l,   qr/Not adding duplicate/, "local-merge: no duplicates");

# merge local into remote
my $output_r = qx{../src/task rc:remote.rc merge local/undo.save 2>&1};

# check output
unlike ($output_r,   qr/Missing/,              "remote-merge: no missing entry");
unlike ($output_r,   qr/Not adding duplicate/, "remote-merge: no duplicates");

# check reports
my $report_l = qx{../src/task rc:local.rc list 2>&1};
my $report_r = qx{../src/task rc:remote.rc list 2>&1};

# local-merge
like   ($report_l,   qr/left_added/,                    "local-merge: left_added is present");
like   ($report_l,   qr/right_added/,                   "local-merge: right_added is present");
like   ($report_l,   qr/H.*left_modified/,              "local-merge: left_modified ok");
like   ($report_l,   qr/\*.*left_newer.*stay/,          "local-merge: left_newer ok, undo-completed");
like   ($report_l,   qr/realProject.*right_newer.*gym/, "local-merge: right_newer ok");

$report_l = qx{../src/task rc:local.rc export 2>&1};
like   ($report_l,   qr/left_deleted.*deleted/,         "local-merge: left_deleted ok");
like   ($report_l,   qr/right_deleted.*deleted/,        "local-merge: right_deleted ok");
like   ($report_l,   qr/left_completed.*completed/,     "local-merge: left_completed ok");
like   ($report_l,   qr/right_completed.*completed/,    "local-merge: right_completed ok");

$report_l = qx{../src/task rc:local.rc waiting 2>&1};
like   ($report_l,   qr/L.*right_modified/,             "local-merge: right_modified ok");

# remote-merge
like   ($report_r,   qr/left_added/,                    "remote-merge: left_added is present");
like   ($report_r,   qr/right_added/,                   "remote-merge: right_added is present");
like   ($report_r,   qr/H.*left_modified/,              "remote-merge: left_modified ok");
like   ($report_r,   qr/\*.*left_newer.*stay/,          "remote-merge: left_newer ok");
like   ($report_r,   qr/realProject.*right_newer.*gym/, "remote-merge: right_newer ok");

$report_r = qx{../src/task rc:remote.rc export 2>&1};
like   ($report_r,   qr/left_deleted.*deleted/,         "remote-merge: left_deleted ok");
like   ($report_r,   qr/right_deleted.*deleted/,        "remote-merge: right_deleted ok");
like   ($report_r,   qr/left_completed.*completed/,     "remote-merge: left_completed ok");
like   ($report_r,   qr/right_completed.*completed/,    "remote-merge: right_completed ok");

$report_r = qx{../src/task rc:remote.rc waiting 2>&1};
like   ($report_r,   qr/L.*right_modified/, "remote-merge: right_modified ok");

# check timestamps in undo.data
my $good = true;
if (open my $fh, 'local/undo.data') {
	my $lasttime = 0;
	while (!eof($fh)) {
		if (defined ($_ = <$fh>)) {
			if ($_ =~ m/^time (\d+)/) {
  			   my $time = $1 + 0;
				if ($time <= $lasttime) {
					fail ("timestamps in local/undo.data are not monotonically ordered");
					$good = false;
				}
				$lasttime = $time;
			}
		}
	}
	close $fh;
} else {
	fail ("could not open local/undo.data");
}
ok ($good, "local-merge: timestamps ok");

$good = true;
if (open my $fh, 'remote/undo.data') {
	my $lasttime = 0;
	while (!eof($fh)) {
		if (defined ($_ = <$fh>)) {
			if ($_ =~ m/^time (\d+)/) {
  			   my $time = $1 + 0;
				if ($time <= $lasttime) {
					fail ("timestamps in remote/undo.data are not monotonically ordered");
					$good = false;
				}
				$lasttime = $time;
			}
		}
	}
	close $fh;
} else {
	fail ("could not open remote/undo.data");
}
ok ($good, "remote-merge: timestamps ok");

# Cleanup.
unlink qw(local/pending.data local/completed.data local/undo.data local/undo.save local/backlog.data local/synch.key local.rc);
ok (! -r 'local/pending.data'   &&
    ! -r 'local/completed.data' &&
    ! -r 'local/undo.data'      &&
    ! -r 'local/undo.save'      &&
    ! -r 'local/backlog.data'   &&
    ! -r 'local/synch.key'      &&
    ! -r 'local.rc', 'Local Cleanup');

unlink qw(remote/pending.data remote/completed.data remote/undo.data remote/backlog.data remote/synch.key remote.rc);
ok (! -r 'remote/pending.data'   &&
    ! -r 'remote/completed.data' &&
    ! -r 'remote/undo.data'      &&
    ! -r 'remote/backlog.data'   &&
    ! -r 'remote/synch.key'      &&
    ! -r 'remote.rc', 'Remove Cleanup');

rmtree (['remote/extensions', 'remote', 'local/extensions', 'local'], 0, 1);
ok (! -e 'remote/extensions' &&
    ! -e 'remote'            &&
    ! -e 'local/extensions'  &&
    ! -e 'local', 'Removed dir local');

exit 0;

