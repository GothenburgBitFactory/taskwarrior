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
use Test::More tests => 9;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'import.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/Y\n";
  close $fh;
}

# Create import file.
if (open my $fh, '>', 'import.txt')
{
  print $fh <<EOF;
{"uuid":"00000000-0000-0000-0000-000000000000","description":"zero","project":"A","status":"pending","entry":"1234567889"}
{"uuid":"11111111-1111-1111-1111-111111111111","description":"one","project":"B","status":"pending","entry":"1234567889"}
{"uuid":"22222222-2222-2222-2222-222222222222","description":"two","status":"completed","entry":"1234524689","end":"1234524690"}
EOF
  close $fh;
}

my $output = qx{../src/task rc:import.rc import import.txt 2>&1 >/dev/null};
like ($output, qr/Imported 3 tasks\./, 'no errors');
# Imported 3 tasks successfully.

$output = qx{../src/task rc:import.rc list 2>&1};
# ID Project Pri Due Active Age     Description
# -- ------- --- --- ------ ------- -----------
#  1 A                      1.5 yrs zero
#  2 B                      1.5 yrs one
#
# 2 tasks

like   ($output, qr/1.+A.+zero/, 't1 present');
like   ($output, qr/2.+B.+one/,  't2 present');
unlike ($output, qr/3.+two/,     't3 missing');

$output = qx{../src/task rc:import.rc completed 2>&1};
# Complete  Project Pri Age     Description
# --------- ------- --- ------- -----------
# 2/13/2009             1.5 yrs two
#
# 1 task

unlike ($output, qr/1.+A.+zero/,       't1 missing');
unlike ($output, qr/2.+B.+one/,        't2 missing');
like   ($output, qr/2\/13\/2009.+two/, 't3 present');

# Make sure that a duplicate task cannot be imported.
$output = qx{../src/task rc:import.rc import import.txt 2>&1 >/dev/null};
like ($output, qr/Cannot add task because the uuid .+ is not unique\./, 'error on duplicate uuid');

# Create import file.
if (open my $fh, '>', 'import2.txt')
{
  print $fh <<EOF;
{"uuid":"44444444-4444-4444-4444-444444444444","description":"three","status":"pending","entry":"1234567889"}
EOF
  close $fh;
}

$output = qx{../src/task rc:import.rc import import2.txt 2>&1 >/dev/null};
like ($output, qr/Imported 1 tasks\./, 'no errors');
# Imported 1 tasks successfully.

# Cleanup.
unlink qw(import.txt import2.txt pending.data completed.data undo.data backlog.data  import.rc);
exit 0;

