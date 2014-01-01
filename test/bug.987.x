#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2014, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 9;

# Create the rc file.
if (open my $fh, '>', 'bug.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'bug.rc', 'Created bug.rc');
}

# Bug 987 - 'Total active time' is still increasing when a task is done

# Test all cases for which the active duration must be close to zero and
# constant:
# - start (a), stop (o), done (d) and none (n)
#   (none disallows the name of each task to be the prefix of any other)
qx{../src/task rc:bug.rc add test_aodn};
qx{../src/task rc:bug.rc test_aodn start};
qx{faketime -f '+1s' ../src/task rc:bug.rc test_aodn stop};
qx{faketime -f '+2s' ../src/task rc:bug.rc test_aodn done};
check_constant_close_zero("test_aodn");

# - start and done
qx{../src/task rc:bug.rc add test_adn};
qx{../src/task rc:bug.rc test_adn start};
qx{faketime -f '+1s' ../src/task rc:bug.rc test_adn done};
check_constant_close_zero("test_adn");

# - start, done and stop
qx{../src/task rc:bug.rc add test_adon};
qx{../src/task rc:bug.rc test_adon start};
qx{faketime -f '+1s' ../src/task rc:bug.rc test_adon done};
qx{faketime -f '+2s' ../src/task rc:bug.rc test_adon stop};
check_constant_close_zero("test_adon");

# - done, start and stop (XXX whether this should be allowed is questionable)
qx{../src/task rc:bug.rc add test_daon};
qx{../src/task rc:bug.rc test_daon done};
qx{faketime -f '+1s' ../src/task rc:bug.rc test_daon start};
qx{faketime -f '+2s' ../src/task rc:bug.rc test_daon stop};
check_constant_close_zero("test_daon");

# Test all cases for which the active duration must not be shown:
# - done
qx{../src/task rc:bug.rc add test_dn};
qx{../src/task rc:bug.rc test_dn done};
check_no_summary("test_dn");

# Test all case for which the active duration must be close to zero and
# increasing:
# - start
qx{../src/task rc:bug.rc add test_sn};
qx{../src/task rc:bug.rc test_sn start};
check_running_close_zero("test_sn");

# - done and start (XXX whether this should be allowed is questionable)
qx{../src/task rc:bug.rc add test_dsn};
qx{../src/task rc:bug.rc test_dsn start};
check_running_close_zero("test_dsn");

### Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data bug.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'bug.rc', 'Cleanup');

exit 0;

################################################################################
# Check that the active time is not above one minute after waiting for one minute
sub check_constant_close_zero {
  my ($task) = @_;
  my $output = qx{faketime -f '+60s' ../src/task rc:bug.rc $task info};
  like ($output, qr/Total active time 0:00:/ms, 'Total active duration is not increasing and is close to zeo');
}

# Check that there is no active time summary
sub check_no_summary {
  my ($task) = @_;
  my $output = qx{faketime -f '+10s' ../src/task rc:bug.rc $task info};
  unlike ($output, qr/Total active time/ms, 'No total active duration summary');
}

# Check that the active time is above one minute after waiting for one minute
sub check_running_close_zero {
  my ($task) = @_;
  my $output = qx{faketime -f '+60s' ../src/task rc:bug.rc $task info};
  like ($output, qr/Total active time 0:01:/ms, 'Total active duration is increasing and is close to zero');
}
