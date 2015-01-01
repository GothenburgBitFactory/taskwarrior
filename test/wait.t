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
use Test::More tests => 7;

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
            "report.ls.columns=id,project,priority,description\n",
            "report.ls.labels=ID,Proj,Pri,Description\n",
            "report.ls.sort=priority-,project+\n",
            "report.ls.filter=status:pending\n",
            "confirmation=off\n";
  close $fh;
}

# [1] add waited task that already expired
# [2] add waited task that is still waited
if (open my $fh, '>', 'pending.data')
{
  my $wait = time () - 3600;     # Became visible an hour ago
  my $entry = $wait - 86400;     # Created a day ago
  my $tomorrow = $wait + 86400;  # Still waiting

  print $fh <<eof;
[uuid:"00000000-0000-0000-0000-000000000000" status:"waiting" description:"visible" entry:"$entry" wait:"$wait"]
[uuid:"00000000-0000-0000-0000-000000000001" status:"waiting" description:"invisible" entry:"$entry" wait:"$tomorrow"]
eof
  close $fh;
}

# verify [1] visible already
# verify [2] inviѕible
my $output = qx{../src/task rc:$rc ls 2>&1};
ok ($? == 0, "$ut: list tasks");
like ($output, qr/visible/ms, "$ut: task 1 visible");
unlike ($output, qr/invisible/ms, "$ut: task 2 invisible");

# verify [1] has status:pending
$output = qx{../src/task rc:$rc 1 info 2>&1};
ok ($? == 0, "$ut: 1 info");
like ($output, qr/Status\s+Pending/ms, "$ut: task 1 pending");

# verify [2] visible via info
$output = qx{../src/task rc:$rc 2 info 2>&1};
ok ($? == 0, "$ut: 2 info");
like ($output, qr/invisible/ms, "$ut: task 2 visible");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;
