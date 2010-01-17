#! /usr/bin/perl
################################################################################
## task - a command line task list manager.
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
use Test::More tests => 37;

# Create the rc file.
if (open my $fh, '>', 'annotate.rc')
{
  # Note: Use 'rrr' to guarantee a unique report name.  Using 'r' conflicts
  #       with 'recurring'.
  print $fh "data.location=.\n",
            "confirmation=off\n",
            "report.rrr.description=rrr\n",
            "report.rrr.columns=id,description\n",
            "report.rrr.sort=id+\n";
  close $fh;
  ok (-r 'annotate.rc', 'Created annotate.rc');
}

# Add four tasks, annotate one three times, one twice, one just once and one none.
qx{../task rc:annotate.rc add one};
qx{../task rc:annotate.rc add two};
qx{../task rc:annotate.rc add three};
qx{../task rc:annotate.rc add four};
qx{../task rc:annotate.rc annotate 1 foo1};
sleep 1;
qx{../task rc:annotate.rc annotate 1 foo2};
sleep 1;
qx{../task rc:annotate.rc annotate 1 foo3};
sleep 1;
qx{../task rc:annotate.rc annotate 2 bar1};
sleep 1;
qx{../task rc:annotate.rc annotate 2 bar2};
sleep 1;
qx{../task rc:annotate.rc annotate 3 baz1};

my $output = qx{../task rc:annotate.rc rrr};

# ID Description                    
# -- -------------------------------
#  1 one
#    3/24/2009 foo1
#    3/24/2009 foo2
#    3/24/2009 foo3
#  2 two
#    3/24/2009 bar1
#    3/24/2009 bar2
#  3 three
#    3/24/2009 baz1
#  4 four
# 
# 4 tasks
like ($output, qr/1 one/,   'task 1');
like ($output, qr/2 two/,   'task 2');
like ($output, qr/3 three/, 'task 3');
like ($output, qr/4 four/,  'task 4');
like ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} foo1/ms,  'first  annotation task 1');
like ($output, qr/foo1.+\d{1,2}\/\d{1,2}\/\d{4} foo2/ms, 'second annotation task 1');
like ($output, qr/foo2.+\d{1,2}\/\d{1,2}\/\d{4} foo3/ms, 'third  annotation task 1');
like ($output, qr/two.+\d{1,2}\/\d{1,2}\/\d{4} bar1/ms,  'first  annotation task 2');
like ($output, qr/bar1.+\d{1,2}\/\d{1,2}\/\d{4} bar2/ms, 'second annotation task 2');
like ($output, qr/three.+\d{1,2}\/\d{1,2}\/\d{4} baz1/ms,'first  annotation task 3');
like ($output, qr/4 tasks/, 'count');

$output = qx{../task rc:annotate.rc rc.annotation.details:1 rrr};
like   ($output, qr/1 \+one/, 'task 1');
like   ($output, qr/2 \+two/, 'task 2');
like   ($output, qr/3 three/, 'task 3');
like   ($output, qr/4 four/,  'task 4');
unlike ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} foo1/ms,   'first  annotation task 1');
unlike ($output, qr/foo1.+\d{1,2}\/\d{1,2}\/\d{4} foo2/ms,  'second annotation task 1');
like   ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} foo3/ms,   'third  annotation task 1');
unlike ($output, qr/two.+\d{1,2}\/\d{1,2}\/\d{4} bar1/ms,   'first  annotation task 2');
like   ($output, qr/two.+\d{1,2}\/\d{1,2}\/\d{4} bar2/ms,   'second annotation task 2');
like   ($output, qr/three.+\d{1,2}\/\d{1,2}\/\d{4} baz1/ms, 'third  annotation task 3');
like   ($output, qr/4 tasks/, 'count');

$output = qx{../task rc:annotate.rc rc.annotation.details:0 rrr};
like   ($output, qr/1 \+one/,   'task 1');
like   ($output, qr/2 \+two/,   'task 2');
like   ($output, qr/3 \+three/, 'task 3');
like   ($output, qr/4 four/,    'task 4');
unlike ($output, qr/one.+\d{1,2}\/\d{1,2}\/\d{4} foo1/ms,  'first  annotation task 1');
unlike ($output, qr/foo1.+\d{1,2}\/\d{1,2}\/\d{4} foo2/ms,  'second annotation task 1');
unlike ($output, qr/foo2.+\d{1,2}\/\d{1,2}\/\d{4} foo3/ms,  'third  annotation task 1');
unlike ($output, qr/two.+\d{1,2}\/\d{1,2}\/\d{4} bar1/ms,   'first  annotation task 2');
unlike ($output, qr/bar1.+\d{1,2}\/\d{1,2}\/\d{4} bar2/ms,  'second annotation task 2');
unlike ($output, qr/three.+\d{1,2}\/\d{1,2}\/\d{4} baz1/ms, 'third  annotation task 3');
like ($output, qr/4 tasks/, 'count');

# Cleanup.
unlink 'pending.data';
ok (!-r 'pending.data', 'Removed pending.data');

unlink 'undo.data';
ok (!-r 'undo.data', 'Removed undo.data');

unlink 'annotate.rc';
ok (!-r 'annotate.rc', 'Removed annotate.rc');

exit 0;

