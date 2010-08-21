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
use Test::More tests => 49;

# Create the rc file.
if (open my $fh, '>', 'urgency.rc')
{
  print $fh "data.location=.\n",
            "urgency.priority.coefficient=10\n",
            "urgency.active.coefficient=10\n",
            "urgency.project.coefficient=10\n",
            "urgency.due.coefficient=10\n",
            "urgency.blocking.coefficient=10\n",
            "urgency.blocked.coefficient=10\n",
            "urgency.annotations.coefficient=10\n",
            "urgency.tags.coefficient=10\n",
            "urgency.waiting.coefficient=10\n",
            "urgency.next.coefficient=10\n",
            "urgency.user.project.PROJECT.coefficient=10\n",
            "urgency.user.tag.TAG.coefficient=10\n",
            "confirmation=off\n";

  close $fh;
  ok (-r 'urgency.rc', 'Created urgency.rc');
}

# Add a task, and verify that the individual urgency terms are being correctly
# calculated.

# priority
qx{../task rc:urgency.rc add control};                   # 1
qx{../task rc:urgency.rc add 1a pri:H};                  # 2
qx{../task rc:urgency.rc add 1b pri:M};                  # 3
qx{../task rc:urgency.rc add 1c pri:L};                  # 4

# priority: 10 (pending) + 10 (unblocked)
my $output = qx{../task rc:urgency.rc 1 _urgency};
like ($output, qr/urgency 20$/ms, 'Control = 20');

# priority: 10 (pri:H) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 2 _urgency};
like ($output, qr/urgency 30$/ms, 'pri:H = 30');

# priority: 6.5 (pri:M) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 3 _urgency};
like ($output, qr/urgency 26\.5$/ms, 'pri:M = 26.5');

# priority: 3 (pri:L) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 4 _urgency};
like ($output, qr/urgency 23$/ms, 'pri:L = 23');

# project: 10 (project) + 10 (pending) + 10 (unblocked)
qx{../task rc:urgency.rc add 2a project:P};              # 5
$output = qx{../task rc:urgency.rc 5 _urgency};
like ($output, qr/urgency 30$/ms, 'pro:P = 30');

# active: 10 (active) + 10 (pending) + 10 (unblocked)
qx{../task rc:urgency.rc add 3a};                        # 6
qx{../task rc:urgency.rc 6 start};
$output = qx{../task rc:urgency.rc 6 _urgency};
like ($output, qr/urgency 30$/ms, 'active = 30');

# next: 10 (+next) + 8 (1 tag) + 10 (pending) + 10 (unblocked)
qx{../task rc:urgency.rc add 4a +next};                  # 7
$output = qx{../task rc:urgency.rc 7 _urgency};
like ($output, qr/urgency 38$/ms, '+next = 38');

# tags
qx{../task rc:urgency.rc add 5a +one};                   # 8
qx{../task rc:urgency.rc add 5b +one +two};              # 9
qx{../task rc:urgency.rc add 5c +one +two +three};       # 10
qx{../task rc:urgency.rc add 5d +one +two +three +four}; # 11

# tags: 8 (1 tag) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 8 _urgency};
like ($output, qr/urgency 28$/ms, '+one = 28');

# tags: 9 (2 tags) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 9 _urgency};
like ($output, qr/urgency 29$/ms, '+one +two = 29');

# tags: 10 (3 tags) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 10 _urgency};
like ($output, qr/urgency 30$/ms, '+one +two +three = 30');

# tags: 10 (4 tags) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 10 _urgency};
like ($output, qr/urgency 30$/ms, '+one +two +three +four = 30');

# annotations
qx{../task rc:urgency.rc add 6a};                        # 12
qx{../task rc:urgency.rc 12 annotate A};
qx{../task rc:urgency.rc add 6b};                        # 13
qx{../task rc:urgency.rc 13 annotate A};
diag ("6 second delay");
sleep 1;
qx{../task rc:urgency.rc 13 annotate B};
qx{../task rc:urgency.rc add 6c};                        # 14
qx{../task rc:urgency.rc 14 annotate A};
sleep 1;
qx{../task rc:urgency.rc 14 annotate B};
sleep 1;
qx{../task rc:urgency.rc 14 annotate C};
qx{../task rc:urgency.rc add 6d};                        # 15
qx{../task rc:urgency.rc 15 annotate A};
sleep 1;
qx{../task rc:urgency.rc 15 annotate B};
sleep 1;
qx{../task rc:urgency.rc 15 annotate C};
sleep 1;
qx{../task rc:urgency.rc 15 annotate D};

# annotations: 8 (1 annotation) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 12 _urgency};
like ($output, qr/urgency 28$/ms, '1 annotation = 28');

# annotations: 9 (2 annotations) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 13 _urgency};
like ($output, qr/urgency 29$/ms, '2 annotations = 29');

# annotations: 10 (3 annotations) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 14 _urgency};
like ($output, qr/urgency 30$/ms, '3 annotations = 30');

# annotations: 10 (4 annotations) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 15 _urgency};
like ($output, qr/urgency 30$/ms, '4 annotations = 30');

# waiting: 10 (unblocked)
qx{../task rc:urgency.rc add 7a wait:10s};               # 16
$output = qx{../task rc:urgency.rc 16 _urgency};
like ($output, qr/urgency 10$/ms, 'waiting = 10');

# blocked: 10 (pending)
qx{../task rc:urgency.rc add 8a depends:1};              # 17
$output = qx{../task rc:urgency.rc 17 _urgency};
like ($output, qr/urgency 10$/ms, 'blocked = 10');

# blocking: 10 (blocking) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 1 _urgency};
like ($output, qr/urgency 30$/ms, 'blocking = 30');

# due
#
#  days overdue, capped at 7     ->  0.8 - 1.0
#  due today                     ->  0.7
#  days until due, capped at 14  ->  0.4 - 0.6
#  has due date                  ->  0.3
#  no due date                   ->  0.0

qx{../task rc:urgency.rc add 9a due:-10d};               # 18
qx{../task rc:urgency.rc add 9b due:-7d};                # 19
qx{../task rc:urgency.rc add 9c due:-6d};                # 20
qx{../task rc:urgency.rc add 9d due:-5d};                # 21
qx{../task rc:urgency.rc add 9e due:-4d};                # 22
qx{../task rc:urgency.rc add 9f due:-3d};                # 23
qx{../task rc:urgency.rc add 9g due:-2d};                # 24
qx{../task rc:urgency.rc add 9h due:-1d};                # 25
qx{../task rc:urgency.rc add 9i due:now};                # 26
qx{../task rc:urgency.rc add 9j due:25h};                # 27
qx{../task rc:urgency.rc add 9k due:49h};                # 28
qx{../task rc:urgency.rc add 9l due:73h};                # 29
qx{../task rc:urgency.rc add 9m due:97h};                # 30
qx{../task rc:urgency.rc add 9n due:121h};               # 31
qx{../task rc:urgency.rc add 9o due:145h};               # 32
qx{../task rc:urgency.rc add 9p due:169h};               # 33
qx{../task rc:urgency.rc add 9q due:193h};               # 34
qx{../task rc:urgency.rc add 9r due:217h};               # 35
qx{../task rc:urgency.rc add 9s due:241h};               # 36
qx{../task rc:urgency.rc add 9t due:265h};               # 37
qx{../task rc:urgency.rc add 9u due:289h};               # 38
qx{../task rc:urgency.rc add 9v due:313h};               # 39
qx{../task rc:urgency.rc add 9w due:337h};               # 40
qx{../task rc:urgency.rc add 9x due:361h};               # 41

# due: 10 (due:-10d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 18 _urgency};
like ($output, qr/urgency 30$/ms, 'due:-10d = 30');

# due: 10 (due:-7d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 19 _urgency};
like ($output, qr/urgency 30$/ms, 'due:-7d = 30');

# due: 9.6 (due:-6d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 20 _urgency};
like ($output, qr/urgency 29.6/ms, 'due:-6d = 29.6');

# due: 9.2 (due:-5d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 21 _urgency};
like ($output, qr/urgency 29.2/ms, 'due:-5d = 29.2');

# due: 8.8 (due:-4d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 22 _urgency};
like ($output, qr/urgency 28.8/ms, 'due:-4d = 28.8');

# due: 8.4 (due:-3d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 23 _urgency};
like ($output, qr/urgency 28.4/ms, 'due:-3d = 28.4');

# due: 8 (due:-2d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 24 _urgency};
like ($output, qr/urgency 28/ms, 'due:-2d = 28');

# due: 7.6 (due:-1d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 25 _urgency};
like ($output, qr/urgency 27.6/ms, 'due:-1d = 27.6');

# due: 7.2 (due:now) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 26 _urgency};
like ($output, qr/urgency 27.2$/ms, 'due:now = 27.2');

# due: 6.8 (due:1d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 27 _urgency};
like ($output, qr/urgency 26.8/ms, 'due:1d = 26.8');

# due: 6.4 (due:2d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 28 _urgency};
like ($output, qr/urgency 26.4/ms, 'due:2d = 26.4');

# due: 6 (due:3d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 29 _urgency};
like ($output, qr/urgency 26/ms, 'due:3d = 26');

# due: 5.6 (due:4d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 30 _urgency};
like ($output, qr/urgency 25.6/ms, 'due:4d = 25.6');

# due: 5.2 (due:5d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 31 _urgency};
like ($output, qr/urgency 25.2/ms, 'due:5d = 25.2');

# due: 4.8 (due:6d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 32 _urgency};
like ($output, qr/urgency 24.8/ms, 'due:6d = 24.8');

# due: 4.4 (due:7d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 33 _urgency};
like ($output, qr/urgency 24.4/ms, 'due:7d = 24.4');

# due: 4 (due:8d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 34 _urgency};
like ($output, qr/urgency 24/ms, 'due:8d = 24');

# due: 3.6 (due:9d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 35 _urgency};
like ($output, qr/urgency 23.6/ms, 'due:9d = 23.6');

# due: 3.2 (due:10d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 36 _urgency};
like ($output, qr/urgency 23.2/ms, 'due:10d = 23.2');

# due: 2.8 (due:11d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 37 _urgency};
like ($output, qr/urgency 22.8/ms, 'due:11d = 22.8');

# due: 2.4 (due:12d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 38 _urgency};
like ($output, qr/urgency 22.4/ms, 'due:12d = 22.4');

# due: 2 (due:13d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 39 _urgency};
like ($output, qr/urgency 22/ms, 'due:13d = 22');

# due: 1.6 (due:14d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 40 _urgency};
like ($output, qr/urgency 21.6/ms, 'due:14d = 21.6');

# due: 1.6 (due:20d) + 10 (pending) + 10 (unblocked)
$output = qx{../task rc:urgency.rc 41 _urgency};
like ($output, qr/urgency 21.6$/ms, 'due:20d = 21.6');

# user.project: 10 (pro:PROJECT) + 10 (project) + 10 (pending) + 10 (unblocked)
qx{../task rc:urgency.rc add 10a project:PROJECT};        # 42
$output = qx{../task rc:urgency.rc 42 _urgency};
like ($output, qr/urgency 40$/ms, 'pro:PROJECT = 40');

# user.tag: 10 (+TAG) + 8 (1 tag) + 10 (pending) + 10 (unblocked)
qx{../task rc:urgency.rc add 11a +TAG};                   # 43
$output = qx{../task rc:urgency.rc 43 _urgency};
like ($output, qr/urgency 38$/ms, '+TAG = 38');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'completed.data';
ok (!-r 'completed.data', 'Removed completed.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'urgency.rc';
ok (!-r 'urgency.rc', 'Removed urgency.rc');

exit 0;
