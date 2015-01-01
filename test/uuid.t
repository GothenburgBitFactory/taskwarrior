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
use Test::More tests => 22;

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
            "dateformat=m/d/Y\n",
            "report.unittest.columns=id,entry,start,description\n",
            "report.unittest.filter=status:pending\n",
            "report.unittest.sort=id\n",
            "confirmation=off\n";
  close $fh;
}

if (open my $fh, '>', 'pending.data')
{
  print $fh <<EOF;
[description:"one" entry:"1315260230" status:"pending" uuid:"9deed7ca-843d-4259-b2c4-40ce73e8e4f3"]
[description:"two" entry:"1315260230" status:"pending" uuid:"0f4c83d2-552f-4108-ae3f-ccc7959f84a3"]
[description:"three" entry:"1315260230" status:"pending" uuid:"aa4abef1-1dc5-4a43-b6a0-7872df3094bb"]
[description:"ssttaarrtt" entry:"1315335826" start:"1315338535" status:"pending" uuid:"d71d3566-7a6b-4c32-8f0b-6de75bb9397b"]
EOF
  close $fh;
}

if (open my $fh, '>', 'completed.data')
{
  print $fh <<EOF;
[description:"four" end:"1315260230" entry:"1315260230" status:"completed" uuid:"ea3b4822-574c-464b-8025-7f7be9f3cc57"]
[description:"five" end:"1315260230" entry:"1315260230" status:"completed" uuid:"0f38b97e-3081-4e75-a1be-65ed3712ea4d"]
[description:"eenndd" end:"1315335841" entry:"1315335841" start:"1315338516" status:"completed" uuid:"727baa6c-65b8-485e-a810-e133e3cd83dc"]
[description:"UUNNDDOO" end:"1315338626" entry:"1315338626" status:"completed" uuid:"c1361003-948e-43e8-85c8-15d28dc3c71c"]
EOF
  close $fh;
}

qx{../src/task rc:$rc 9deed7ca-843d-4259-b2c4-40ce73e8e4f3 modify ONE 2>&1};
qx{../src/task rc:$rc 2 modify TWO 2>&1};
my $output = qx{../src/task rc:$rc list 2>&1};
like ($output, qr/ONE/, 'task list ONE');
like ($output, qr/TWO/, 'task list TWO');
like ($output, qr/three/, 'task list three');
like ($output, qr/ssttaarrtt/, 'task list ssttaarrtt');
unlike ($output, qr/four/, 'task does not list four');
unlike ($output, qr/five/, 'task does not list five');
unlike ($output, qr/eenndd/, 'task does not list eenndd');
unlike ($output, qr/UUNNDDOO/, 'task does not list UUNNDDOO');

qx{../src/task rc:$rc ea3b4822-574c-464b-8025-7f7be9f3cc57 modify FOUR 2>&1};
$output = qx{../src/task rc:$rc completed 2>&1};
unlike ($output, qr/ONE/, 'task does not list ONE');
unlike ($output, qr/TWO/, 'task does not list TWO');
unlike ($output, qr/three/, 'task does not list three');
unlike ($output, qr/ssttaarrtt/, 'task does not list ssttaarrtt');
like ($output, qr/FOUR/, 'modified completed task FOUR');
like ($output, qr/five/, 'did not modify task five');
like ($output, qr/eenndd/, 'did not modify task eenndd');
like ($output, qr/UUNNDDOO/, 'did not modify task UUNNDDOO');

qx{../src/task rc:$rc c1361003-948e-43e8-85c8-15d28dc3c71c modify status:pending 2>&1};
$output = qx{../src/task rc:$rc list 2>&1};
like ($output, qr/UUNNDDOO/, 'task UUNNDDOO modified status to pending');
$output = qx{../src/task rc:$rc completed 2>&1};
unlike ($output, qr/UUNNDDOO/, 'task does not list UUNNDDOO after modification');

qx{../src/task rc:$rc d71d3566-7a6b-4c32-8f0b-6de75bb9397b modify start:12/31/2010 2>&1};
$output = qx{../src/task rc:$rc unittest 2>&1};
like ($output, qr/12\/31\/2010/, 'modified start date of task ssttaarrtt');

qx{../src/task rc:$rc 727baa6c-65b8-485e-a810-e133e3cd83dc modify end:12/31/2010 2>&1};
$output = qx{../src/task rc:$rc completed 2>&1};
like ($output, qr/12\/31\/2010/, 'modified end date of task eenndd');

qx{../src/task rc:$rc aa4abef1-1dc5-4a43-b6a0-7872df3094bb modify entry:12/30/2010 2>&1};
qx{../src/task rc:$rc aa4abef1-1dc5-4a43-b6a0-7872df3094bb modify start:1/1/2011 2>&1};
$output = qx{../src/task rc:$rc unittest 2>&1};
like ($output, qr/12\/30\/2010/, 'modified entry date of task three');
like ($output, qr/1\/1\/2011/, 'added start date of task three with modify');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

