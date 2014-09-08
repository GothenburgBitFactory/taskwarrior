#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 47;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

sub in_range
{
  my ($value, $low, $high, $message) = @_;

  if ($value >= $low && $value <= $high)
  {
    pass ($message);
  }
  else
  {
    diag ("Expected '$value' to be in the range $low --> $high");
    fail ($message);
  }
}

# Create the rc file.
if (open my $fh, '>', $rc)
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
            "urgency.waiting.coefficient=-10\n",
            "urgency.next.coefficient=10\n",
            "urgency.user.project.PROJECT.coefficient=10\n",
            "urgency.user.tag.TAG.coefficient=10\n",
            "confirmation=off\n";

  close $fh;
}

# Add a task, and verify that the individual urgency terms are being correctly
# calculated.

# priority
qx{../src/task rc:$rc add control 2>&1};                   # task 1
qx{../src/task rc:$rc add 1a pri:H 2>&1};                  # task 2
qx{../src/task rc:$rc add 1b pri:M 2>&1};                  # task 3
qx{../src/task rc:$rc add 1c pri:L 2>&1};                  # task 4

# priority: 0 (pending)
my $output = qx{../src/task rc:$rc 1 _urgency 2>&1};
like ($output, qr/urgency 0$/ms, "$ut: Control = 0");

# priority: 10 (pri:H)
$output = qx{../src/task rc:$rc 2 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: pri:H = 10");

# priority: 6.5 (pri:M)
$output = qx{../src/task rc:$rc 3 _urgency 2>&1};
like ($output, qr/urgency 6\.5$/ms, "$ut: pri:M = 6.5");

# priority: 3 (pri:L)
$output = qx{../src/task rc:$rc 4 _urgency 2>&1};
like ($output, qr/urgency 3$/ms, "$ut: pri:L = 3");

# project: 10 (project)
qx{../src/task rc:$rc add 2a project:P 2>&1};              # task 5
$output = qx{../src/task rc:$rc 5 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: pro:P = 10");

# active: 10 (active)
qx{../src/task rc:$rc add 3a 2>&1};                        # task 6
qx{../src/task rc:$rc 6 start 2>&1};
$output = qx{../src/task rc:$rc 6 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: active = 10");

# next: 10 (+next) + 8 (1 tag)
qx{../src/task rc:$rc add 4a +next 2>&1};                  # task 7
$output = qx{../src/task rc:$rc 7 _urgency 2>&1};
like ($output, qr/urgency 18$/ms, "$ut: +next = 18");

# tags
qx{../src/task rc:$rc add 5a +one 2>&1};                   # task 8
qx{../src/task rc:$rc add 5b +one +two 2>&1};              # task 9
qx{../src/task rc:$rc add 5c +one +two +three 2>&1};       # task 10
qx{../src/task rc:$rc add 5d +one +two +three +four 2>&1}; # task 11

# tags: 8 (1 tag)
$output = qx{../src/task rc:$rc 8 _urgency 2>&1};
like ($output, qr/urgency 8$/ms, "$ut: +one = 8");

# tags: 9 (2 tags)
$output = qx{../src/task rc:$rc 9 _urgency 2>&1};
like ($output, qr/urgency 9$/ms, "$ut: +one +two = 9");

# tags: 10 (3 tags)
$output = qx{../src/task rc:$rc 10 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: +one +two +three = 10");

# tags: 10 (4 tags)
$output = qx{../src/task rc:$rc 10 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: +one +two +three +four = 10");

# annotations
qx{../src/task rc:$rc add 6a 2>&1};                        # task 12
qx{../src/task rc:$rc 12 annotate A 2>&1};
qx{../src/task rc:$rc add 6b 2>&1};                        # task 13
qx{../src/task rc:$rc 13 annotate A 2>&1};
qx{../src/task rc:$rc 13 annotate B 2>&1};
qx{../src/task rc:$rc add 6c 2>&1};                        # task 14
qx{../src/task rc:$rc 14 annotate A 2>&1};
qx{../src/task rc:$rc 14 annotate B 2>&1};
qx{../src/task rc:$rc 14 annotate C 2>&1};
qx{../src/task rc:$rc add 6d 2>&1};                        # task 15
qx{../src/task rc:$rc 15 annotate A 2>&1};
qx{../src/task rc:$rc 15 annotate B 2>&1};
qx{../src/task rc:$rc 15 annotate C 2>&1};
qx{../src/task rc:$rc 15 annotate D 2>&1};

# annotations: 8 (1 annotation)
$output = qx{../src/task rc:$rc 12 _urgency 2>&1};
like ($output, qr/urgency 8$/ms, "$ut: 1 annotation = 8");

# annotations: 9 (2 annotations)
$output = qx{../src/task rc:$rc 13 _urgency 2>&1};
like ($output, qr/urgency 9$/ms, "$ut: 2 annotations = 9");

# annotations: 10 (3 annotations)
$output = qx{../src/task rc:$rc 14 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: 3 annotations = 10");

# annotations: 10 (4 annotations)
$output = qx{../src/task rc:$rc 15 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: 4 annotations = 10");

# waiting: -10
qx{../src/task rc:$rc add 7a wait:10s 2>&1};               # task 16
$output = qx{../src/task rc:$rc 16 _urgency 2>&1};
like ($output, qr/urgency -10$/ms, "$ut: waiting = -10");

# blocked: 10 (blocked)
qx{../src/task rc:$rc add 8a depends:1 2>&1};              # task 17
$output = qx{../src/task rc:$rc 17 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: blocked = 10");

# blocking: 10 (blocking)
$output = qx{../src/task rc:$rc 1 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: blocking = 10");

# due

qx{../src/task rc:$rc add 9a due:-10d 2>&1};               # task 18
qx{../src/task rc:$rc add 9b due:-7d 2>&1};                # task 19
qx{../src/task rc:$rc add 9c due:-6d 2>&1};                # task 20
qx{../src/task rc:$rc add 9d due:-5d 2>&1};                # task 21
qx{../src/task rc:$rc add 9e due:-4d 2>&1};                # task 22
qx{../src/task rc:$rc add 9f due:-3d 2>&1};                # task 23
qx{../src/task rc:$rc add 9g due:-2d 2>&1};                # task 24
qx{../src/task rc:$rc add 9h due:-1d 2>&1};                # task 25
qx{../src/task rc:$rc add 9i due:now 2>&1};                # task 26
qx{../src/task rc:$rc add 9j due:25h 2>&1};                # task 27
qx{../src/task rc:$rc add 9k due:49h 2>&1};                # task 28
qx{../src/task rc:$rc add 9l due:73h 2>&1};                # task 29
qx{../src/task rc:$rc add 9m due:97h 2>&1};                # task 30
qx{../src/task rc:$rc add 9n due:121h 2>&1};               # task 31
qx{../src/task rc:$rc add 9o due:145h 2>&1};               # task 32
qx{../src/task rc:$rc add 9p due:169h 2>&1};               # task 33
qx{../src/task rc:$rc add 9q due:193h 2>&1};               # task 34
qx{../src/task rc:$rc add 9r due:217h 2>&1};               # task 35
qx{../src/task rc:$rc add 9s due:241h 2>&1};               # task 36
qx{../src/task rc:$rc add 9t due:265h 2>&1};               # task 37
qx{../src/task rc:$rc add 9u due:289h 2>&1};               # task 38
qx{../src/task rc:$rc add 9v due:313h 2>&1};               # task 39
qx{../src/task rc:$rc add 9w due:337h 2>&1};               # task 40
qx{../src/task rc:$rc add 9x due:361h 2>&1};               # task 41

# due: 10 (due:-10d)
$output = qx{../src/task rc:$rc 18 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: due:-10d = 10");

# due: 10 (due:-7d)
$output = qx{../src/task rc:$rc 19 _urgency 2>&1};
like ($output, qr/urgency 10$/ms, "$ut: due:-7d = 10");

# due: ~9.6 (due:-6d)
$output = qx{../src/task rc:$rc 20 _urgency 2>&1};
my ($value) = $output =~ /urgency\s([0-9.]+)/;
in_range ($value, 9, 10, "$ut: due:-6d = 9 - 10");

# due: 9.24 (due:-5d)
$output = qx{../src/task rc:$rc 21 _urgency 2>&1};
like ($output, qr/urgency 9.24/ms, "$ut: due:-5d = 9.24");

# due: 8.86 (due:-4d)
$output = qx{../src/task rc:$rc 22 _urgency 2>&1};
like ($output, qr/urgency 8.86/ms, "$ut: due:-4d = 8.86");

# due: 8.48 (due:-3d)
$output = qx{../src/task rc:$rc 23 _urgency 2>&1};
like ($output, qr/urgency 8.48/ms, "$ut: due:-3d = 8.48");

# due: 8.1 (due:-2d)
$output = qx{../src/task rc:$rc 24 _urgency 2>&1};
like ($output, qr/urgency 8.1/ms, "$ut: due:-2d = 8.1");

# due: 7.71 (due:-1d)
$output = qx{../src/task rc:$rc 25 _urgency 2>&1};
like ($output, qr/urgency 7.71/ms, "$ut: due:-1d = 7.71");

# due: ~7.53 (due:now)
$output = qx{../src/task rc:$rc 26 _urgency 2>&1};
($value) = $output =~ /urgency\s([0-9.]+)/;
in_range ($value, 7, 8, "$ut: due:now = 7 - 8");

# due: 6.94 (due:1d)
$output = qx{../src/task rc:$rc 27 _urgency 2>&1};
like ($output, qr/urgency 6.94/ms, "$ut: due:1d = 6.94");

# due: 6.56 (due:2d)
$output = qx{../src/task rc:$rc 28 _urgency 2>&1};
like ($output, qr/urgency 6.56/ms, "$ut: due:2d = 6.56");

# due: 6.17 (due:3d)
$output = qx{../src/task rc:$rc 29 _urgency 2>&1};
like ($output, qr/urgency 6.17/ms, "$ut: due:3d = 6.17");

# due: 5.79 (due:4d)
$output = qx{../src/task rc:$rc 30 _urgency 2>&1};
like ($output, qr/urgency 5.79/ms, "$ut: due:4d = 5.79");

# due: 5.41 (due:5d)
$output = qx{../src/task rc:$rc 31 _urgency 2>&1};
like ($output, qr/urgency 5.41/ms, "$ut: due:5d = 5.41");

# due: 5.03 (due:6d)
$output = qx{../src/task rc:$rc 32 _urgency 2>&1};
like ($output, qr/urgency 5.03/ms, "$ut: due:6d = 5.03");

# due: 4.65 (due:7d)
$output = qx{../src/task rc:$rc 33 _urgency 2>&1};
like ($output, qr/urgency 4.65/ms, "$ut: due:7d = 4.65");

# due: 4.27 (due:8d)
$output = qx{../src/task rc:$rc 34 _urgency 2>&1};
like ($output, qr/urgency 4.27/ms, "$ut: due:8d = 4.27");

# due: 3.89 (due:9d)
$output = qx{../src/task rc:$rc 35 _urgency 2>&1};
like ($output, qr/urgency 3.89/ms, "$ut: due:9d = 3.89");

# due: 3.51 (due:10d)
$output = qx{../src/task rc:$rc 36 _urgency 2>&1};
like ($output, qr/urgency 3.51/ms, "$ut: due:10d = 3.51");

# due: 3.13 (due:11d)
$output = qx{../src/task rc:$rc 37 _urgency 2>&1};
like ($output, qr/urgency 3.13/ms, "$ut: due:11d = 3.13");

# due: 2.75 (due:12d)
$output = qx{../src/task rc:$rc 38 _urgency 2>&1};
like ($output, qr/urgency 2.75/ms, "$ut: due:12d = 2.75");

# due: >2 (due:13d)
$output = qx{../src/task rc:$rc 39 _urgency 2>&1};
($value) = $output =~ /urgency\s([0-9.]+)/;
in_range ($value, 2, 3, "$ut: due:13d = 2 - 3");

# due: 2 (due:14d)
$output = qx{../src/task rc:$rc 40 _urgency 2>&1};
like ($output, qr/urgency 2/ms, "$ut: due:14d = 2");

# due: 2 (due:20d)
$output = qx{../src/task rc:$rc 41 _urgency 2>&1};
like ($output, qr/urgency 2$/ms, "$ut: due:20d = 2");

# user.project: 10 (pro:PROJECT) + 10 (project)
qx{../src/task rc:$rc add 10a project:PROJECT 2>&1};        # task 42
$output = qx{../src/task rc:$rc 42 _urgency 2>&1};
like ($output, qr/urgency 20$/ms, "$ut: pro:PROJECT = 20");

# user.tag: 10 (+TAG) + 8 (1 tag)
qx{../src/task rc:$rc add 11a +TAG 2>&1};                   # task 43
$output = qx{../src/task rc:$rc 43 _urgency 2>&1};
like ($output, qr/urgency 18$/ms, "$ut: +TAG = 18");

# scheduled 0 (scheduled future)
qx {../src/task rc:$rc add 12a scheduled:30d 2>&1};
$output = qx{../src/task rc:$rc 44 _urgency 2>&1};
like ($output, qr/urgency 0$/ms, "$ut: scheduled future = 0");

# scheduled 5 (scheduled past)
qx {../src/task rc:$rc add 12b scheduled:yesterday 2>&1};
$output = qx{../src/task rc:$rc 45 _urgency 2>&1};
like ($output, qr/urgency 5$/ms, "$ut: scheduled past = 5");

# urgency values between 0 and 1
qx {../src/task rc:$rc add 13 pri:H 2>&1};
$output = qx{../src/task rc:$rc rc.urgency.priority.coefficient:0.01234 46 info 2>&1};
like ($output, qr/Urgency\s+0\.01$/ms, "$ut: near-zero urgency is truncated");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
