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
use Test::More tests => 39;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n";
  close $fh;
}

=pod
http://github.com/pbeckingham/task/blob/857f813a24f7ce15fea9f2c28aadad84cb5c8847/src/task.cpp
619  // If the period is an 'easy' one, add it to current, and we're done.
620  int days = convertDuration (period);

Date getNextRecurrence (Date& current, std::string& period)

starting at line 509 special cases several possibilities for period, '\d\s?m'
'monthly', 'quarterly', 'semiannual', 'bimonthly', 'biannual', 'biyearly'.
Everything else falls through with period being passed to convertDuration.
convertDuration doesn't know about 'daily' though so it seems to be returning 0.

Confirmed:
  getNextRecurrence  convertDuration
  -----------------  ---------------
                     daily
                     day
                     weekly
                     sennight
                     biweekly
                     fortnight
  monthly            monthly
  quarterly          quarterly
  semiannual         semiannual
  bimonthly          bimonthly
  biannual           biannual
  biyearly           biyearly
                     annual
                     yearly
  *m                 *m
  *q                 *q
                     *d
                     *w
                     *y
=cut

my $output = qx{../src/task rc:$rc add daily due:tomorrow recur:daily 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:daily");

$output = qx{../src/task rc:$rc add 1day due:tomorrow recur:1day 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:1day");

$output = qx{../src/task rc:$rc add weekly due:tomorrow recur:weekly 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:weekly");

$output = qx{../src/task rc:$rc add 1sennight due:tomorrow recur:1sennight 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:1sennight");

$output = qx{../src/task rc:$rc add biweekly due:tomorrow recur:biweekly 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:biweekly");

$output = qx{../src/task rc:$rc add fortnight due:tomorrow recur:fortnight 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:fortnight");

$output = qx{../src/task rc:$rc add monthly due:tomorrow recur:monthly 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:monthly");

$output = qx{../src/task rc:$rc add quarterly due:tomorrow recur:quarterly 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:quarterly");

$output = qx{../src/task rc:$rc add semiannual due:tomorrow recur:semiannual 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:semiannual");

$output = qx{../src/task rc:$rc add bimonthly due:tomorrow recur:bimonthly 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:bimonthly");

$output = qx{../src/task rc:$rc add biannual due:tomorrow recur:biannual 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:biannual");

$output = qx{../src/task rc:$rc add biyearly due:tomorrow recur:biyearly 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:biyearly");

$output = qx{../src/task rc:$rc add annual due:tomorrow recur:annual 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:annual");

$output = qx{../src/task rc:$rc add yearly due:tomorrow recur:yearly 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:yearly");

$output = qx{../src/task rc:$rc add 2d due:tomorrow recur:2d 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:2d");

$output = qx{../src/task rc:$rc add 2w due:tomorrow recur:2w 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:2w");

$output = qx{../src/task rc:$rc add 2m due:tomorrow recur:2mo 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:2m");

$output = qx{../src/task rc:$rc add 2q due:tomorrow recur:2q 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:2q");

$output = qx{../src/task rc:$rc add 2y due:tomorrow recur:2y 2>&1};
unlike ($output, qr/was not recognized/, "$ut: recur:2y");

# Verify that the recurring task instances get created.  One of each.
$output = qx{../src/task rc:$rc list 2>&1};
like ($output, qr/\bdaily\b/,      "$ut: verify daily");
like ($output, qr/\b1day\b/,       "$ut: verify 1day");
like ($output, qr/\bweekly\b/,     "$ut: verify weekly");
like ($output, qr/\b1sennight\b/,  "$ut: verify 1sennight");
like ($output, qr/\bbiweekly\b/,   "$ut: verify biweekly");
like ($output, qr/\bfortnight\b/,  "$ut: verify fortnight");
like ($output, qr/\bmonthly\b/,    "$ut: verify monthly");
like ($output, qr/\bquarterly\b/,  "$ut: verify quarterly");
like ($output, qr/\bsemiannual\b/, "$ut: verify semiannual");
like ($output, qr/\bbimonthly\b/,  "$ut: verify bimonthly");
like ($output, qr/\bbiannual\b/,   "$ut: verify biannual");
like ($output, qr/\bbiyearly\b/,   "$ut: verify biyearly");
like ($output, qr/\bannual\b/,     "$ut: verify annual");
like ($output, qr/\byearly\b/,     "$ut: verify yearly");
like ($output, qr/\b2d\b/,         "$ut: verify 2d");
like ($output, qr/\b2w\b/,         "$ut: verify 2w");
like ($output, qr/\b2m\b/,         "$ut: verify 2m");
like ($output, qr/\b2q\b/,         "$ut: verify 2q");
like ($output, qr/\b2y\b/,         "$ut: verify 2y");

$output = qx{../src/task rc:$rc diag 2>&1};
like ($output, qr/No duplicates found/, "$ut: No duplicate UUIDs detected");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

