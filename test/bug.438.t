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
use Test::More tests => 12;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "dateformat=SNHDMY\n",
            "report.foo.columns=entry,start,end,description\n",
            "report.foo.dateformat=SNHDMY\n",
            "confirmation=off\n";
  close $fh;
}

# Bug #438: Reports sorting by end, start, and entry are ordered incorrectly, if
#           time is included.

if (open my $fh, '>', 'pending.data')
{
  my $entry = time () - 3600;  # An hour ago.

  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000001" status:"pending" description:"one older" entry:"$entry"]\n};
  $entry++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000002" status:"pending" description:"one newer" entry:"$entry"]\n};

  my $start = $entry + 10;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000003" status:"pending" description:"two older" entry:"$entry" start:"$start"]\n};
  $start++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000004" status:"pending" description:"two newer" entry:"$entry" start:"$start"]\n};

  my $end = $start + 10;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000005" status:"completed" description:"three older" entry:"$entry" end:"$end"]\n};
  $end++;
  print $fh qq{[uuid:"00000000-0000-0000-0000-000000000006" status:"completed" description:"three newer" entry:"$entry" end:"$end"]\n};

  close $fh;
}

# Sorting by entry +/-
my $output = qx{../src/task rc:$rc rc.report.foo.sort:entry+ foo 2>&1};
ok ($? == 0,                                   "$ut: entry foo");
like ($output, qr/one older.+one newer/ms,     "$ut: sort:entry+ -> older newer");

$output = qx{../src/task rc:$rc rc.report.foo.sort:entry- foo 2>&1};
ok ($? == 0,                                   "$ut: entry foo");
like ($output, qr/one newer.+one older/ms,     "$ut: sort:entry- -> newer older");

# Sorting by start +/-
$output = qx{../src/task rc:$rc rc.report.foo.sort:start+ foo 2>&1};
ok ($? == 0,                                   "$ut: start foo");
like ($output, qr/two older.+two newer/ms,     "$ut: sort:start+ -> older newer");

$output = qx{../src/task rc:$rc rc.report.foo.sort:start- foo 2>&1};
ok ($? == 0,                                   "$ut: start foo");
like ($output, qr/two newer.+two older/ms,     "$ut: sort:start- -> newer older");

# Sorting by end +/-
$output = qx{../src/task rc:$rc rc.report.foo.sort:end+ foo 2>&1};
ok ($? == 0,                                   "$ut: end foo");
like ($output, qr/three older.+three newer/ms, "$ut: sort:end+ -> older newer");

$output = qx{../src/task rc:$rc rc.report.foo.sort:end- foo 2>&1};
ok ($? == 0,                                   "$ut: end foo");
like ($output, qr/three newer.+three older/ms, "$ut: sort:end- -> newer older");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

