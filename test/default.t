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
use Test::More tests => 19;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'default.rc')
{
  print $fh "data.location=.\n",
            "default.command=list\n",
            "default.project=PROJECT\n",
            "uda.priority.default=M\n",
            "default.due=eom\n",
            "dateformat=m/d/Y\n";
  close $fh;
}

# Set up a default command, project and priority.
qx{../src/task rc:default.rc add all defaults 2>&1};
my $output = qx{../src/task rc:default.rc list 2>&1};
like ($output, qr/ all defaults/, 'task added');
like ($output, qr/ PROJECT /,     'default project added');
like ($output, qr/ M /,           'default priority added');
like ($output, qr/\//,            'default due added');
unlink 'pending.data';

qx{../src/task rc:default.rc add project:specific priority:L due:eoy all specified 2>&1};
$output = qx{../src/task rc:default.rc list 2>&1};
like ($output, qr/ all specified/, 'task added');
like ($output, qr/ specific /,     'project specified');
like ($output, qr/ L /,            'priority specified');
like ($output, qr/\//,             'due specified');
unlink 'pending.data';

qx{../src/task rc:default.rc add project:specific project specified 2>&1};
$output = qx{../src/task rc:default.rc list 2>&1};
like ($output, qr/ project specified/, 'task added');
like ($output, qr/ specific /,         'project specified');
like ($output, qr/ M /,                'default priority added');
like ($output, qr/\//,                 'default due added');
unlink 'pending.data';

qx{../src/task rc:default.rc add priority:L priority specified 2>&1};
$output = qx{../src/task rc:default.rc list 2>&1};
like ($output, qr/ priority specified/, 'task added');
like ($output, qr/ PROJECT /,           'default project added');
like ($output, qr/ L /,                 'priority specified');
like ($output, qr/\//,                  'default due added');

$output = qx{../src/task rc:default.rc 2>&1};
like ($output, qr/1 .+ L PROJECT .+ priority specified/, 'default command worked');

qx{../src/task rc:default.rc add project:HOME priority:M due:tomorrow all specified 2>&1};
qx{echo 'y' | ../src/task rc:default.rc config default.command 'list priority:M' 2>&1};
$output = qx{../src/task rc:default.rc 2>&1};
like   ($output, qr/ M /, 'priority:M included in default command');
unlike ($output, qr/ L /, 'priority:L excluded from default command');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data default.rc);
exit 0;

