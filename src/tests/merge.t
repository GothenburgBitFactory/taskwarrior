#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
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
use Test::More tests => 42;
use File::Copy;

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
				"report.list.columns=id,project,active,priority,description,tags\n",
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
				"report.list.columns=id,project,active,priority,description,tags\n",
				"report.list.labels=id,pro,a,pri,d,t\n",
				"report.list.sort=id+\n",
				"report.list.filter=status:pending\n";
  close $fh;
  ok (-r 'remote.rc', 'Created remote.rc');
}

# Create some basic tasks on both sides
qx{../task rc:local.rc add left_modified};
diag ("25 second delay");
sleep(1);
qx{../task rc:local.rc add right_modified};
sleep(1);
qx{../task rc:local.rc add left_newer};
sleep(1);
qx{../task rc:local.rc add right_newer};
sleep(1);
qx{../task rc:local.rc add left_deleted};
sleep(1);
qx{../task rc:local.rc add right_deleted};
sleep(1);
qx{../task rc:local.rc add left_completed};
sleep(1);
qx{../task rc:local.rc add right_completed};
sleep(1);

copy("local/undo.data", "remote/undo.data") or fail("copy local/undo.data to remote/undo.data");
copy("local/pending.data", "remote/pending.data") or fail("copy local/undo.data to remote/undo.data");
copy("local/completed.data", "remote/completed.data") or fail("copy local/undo.data to remote/undo.data");

# make local modifications
qx{../task rc:local.rc add left_added}; #left_added
sleep(1);
qx{../task rc:local.rc 1 prio:H};       #left_modified
sleep(1);
qx{../task rc:local.rc 3 +stay};        #left_newer
sleep(1);
qx{../task rc:local.rc 4 project:test}; #right_newer
sleep(1);
qx{../task rc:local.rc 6 +delete};      #right_deleted
sleep(1);

# make remote modifications
qx{../task rc:remote.rc add right_added};    #right_added
sleep(1);
qx{../task rc:remote.rc 2 prio:L};           #right_modified
sleep(1);
qx{../task rc:remote.rc 2 wait:tomorrow};    #right_modified
sleep(1);
qx{../task rc:remote.rc 4 proj:realProject}; #right_newer
sleep(1);
qx{../task rc:remote.rc 5 project:deletion}; #left_deleted
sleep(1);
qx{../task rc:remote.rc done 8};             #right_completed
sleep(1);
qx{../task rc:remote.rc del 6};              #right_deleted
sleep(1);
qx{../task rc:remote.rc done 3};             #left_newer
sleep(1);

# make new local modifications
qx{../task rc:local.rc start 3};         #left_newer
sleep(1);
qx{../task rc:local.rc 4 +car};          #right_newer
sleep(1);
qx{../task rc:local.rc done 7};          #left_completed
sleep(1);
qx{../task rc:local.rc del 5};           #left_deleted
sleep(1);

# make new remote modifications
qx{../task rc:remote.rc 4 +gym};         # right_newer

# merge remote into local
my $output_l = qx{../task rc:local.rc merge remote/undo.data};

#check output
like   ($output_l,   qr/Running redo/,         "local-merge finished");
unlike ($output_l,   qr/Missing/,              "local-merge: no missing entry");
unlike ($output_l,   qr/Not adding duplicate/, "local-merge: no duplicates");

# merge local into remote
my $output_r = qx{../task rc:remote.rc merge local/undo.data};

# check output
like   ($output_r,   qr/Running redo/,         "remote-merge finished");
unlike ($output_r,   qr/Missing/,              "remote-merge: no missing entry");
unlike ($output_r,   qr/Not adding duplicate/, "remote-merge: no duplicates");

# check reports
my $report_l = qx{../task rc:local.rc};
my $report_r = qx{../task rc:remote.rc};

# local-merge
like   ($report_l,   qr/left_added/,       "local-merge: left_added is present");
like   ($report_l,   qr/right_added/,      "local-merge: right_added is present");
like   ($report_l,   qr/H.*left_modified/, "local-merge: left_modified ok");
like   ($report_l,   qr/\*.*left_newer.*stay/, "local-merge: left_newer ok");
like   ($report_l,   qr/realProject.*right_newer.*gym/, "local-merge: right_newer ok");

$report_l = qx{../task rc:local.rc export.csv};
like   ($report_l,   qr/deleted.*left_deleted/,      "local-merge: left_deleted ok");
like   ($report_l,   qr/deleted.*right_deleted/,     "local-merge: right_deleted ok");
like   ($report_l,   qr/completed.*left_completed/,  "local-merge: left_completed ok");
like   ($report_l,   qr/completed.*right_completed/,  "local-merge: right_completed ok");

$report_l = qx(../task rc:local.rc waiting);
like   ($report_l,   qr/L.*right_modified/, "local-merge: right_modified ok");

# remote-merge
like   ($report_r,   qr/left_added/,       "remote-merge: left_added is present");
like   ($report_r,   qr/right_added/,      "remote-merge: right_added is present");
like   ($report_r,   qr/H.*left_modified/, "remote-merge: left_modified ok");
like   ($report_r,   qr/\*.*left_newer.*stay/, "remote-merge: left_newer ok");
like   ($report_r,   qr/realProject.*right_newer.*gym/, "remote-merge: right_newer ok");

$report_r = qx{../task rc:remote.rc export.csv};
like   ($report_r,   qr/deleted.*left_deleted/,      "remote-merge: left_deleted ok");
like   ($report_r,   qr/deleted.*right_deleted/,     "remote-merge: right_deleted ok");
like   ($report_r,   qr/completed.*left_completed/,  "remote-merge: left_completed ok");
like   ($report_r,   qr/completed.*right_completed/,  "remote-merge: right_completed ok");

$report_r = qx(../task rc:remote.rc waiting);
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
} else {
	fail ("could not open remote/undo.data");
}
ok ($good, "remote-merge: timestamps ok");

# Cleanup.
unlink 'local/pending.data';
ok (!-r 'local/pending.data', 'Removed local/pending.data');

unlink 'local/completed.data';
ok (!-r 'local/completed.data', 'Removed local/completed.data');

unlink 'local/undo.data';
ok (!-r 'local/undo.data', 'Removed local/undo.data');

unlink 'local.rc';
ok (!-r 'local.rc', 'Removed local.rc');

unlink 'remote/pending.data';
ok (!-r 'remote/pending.data', 'Removed remote/pending.data');

unlink 'remote/completed.data';
ok (!-r 'remote/completed.data', 'Removed remote/completed.data');

unlink 'remote/undo.data';
ok (!-r 'remote/undo.data', 'Removed remote/undo.data');

unlink 'remote.rc';
ok (!-r 'remote.rc', 'Removed remote.rc');

rmdir("remote");
ok (!-e "remote", "Removed dir remote");
rmdir("local");
ok (!-e "local", "Removed dir local");

exit 0;

