#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
if (open my $fh, '>', 'bug.1056')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'bug.1056', 'Created pro.rc');
}

# Bug 1056: Project indentation in CmdSummary.
qx{../src/task rc:bug.1056 add testing project:existingParent 2>&1 >/dev/null};
qx{../src/task rc:bug.1056 add testing project:existingParent.child 2>&1 >/dev/null};
qx{../src/task rc:bug.1056 add testing project:abstractParent.kid 2>&1 >/dev/null};
qx{../src/task rc:bug.1056 add testing project:.myProject 2>&1 >/dev/null};
qx{../src/task rc:bug.1056 add testing project:myProject. 2>&1 >/dev/null};
qx{../src/task rc:bug.1056 add testing project:.myProject. 2>&1 >/dev/null};

my $output = qx{../src/task rc:bug.1056 summary 2>&1};
my @lines = split ('\n',$output);

# It's easier to make a pattern for the end than the beginning because
# project names can have spaces.
my $project_name_column;
if ($lines[4] =~ s/\s+\d+\s+-\s+\d+%$//)
{
  $project_name_column = $lines[4];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^\.myProject\s*$/, '\'.myProject\' not indented');

if ($lines[5] =~ s/\s+\d+\s+-\s+\d+%$//)
{
  $project_name_column = $lines[5];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^\.myProject\.\s*$/, '\'.myProject.\' not indented');

if ($lines[6] =~ s/\s+\d+\s+-\s+\d+%$//)
{
  $project_name_column = "error";
}
else
{
  $project_name_column = $lines[6];
}
like ($project_name_column, qr/^abstractParent\s*$/, 'abstract parent not indented and no priority columns');

if ($lines[7] =~ s/\s+\d+\s+-\s+\d+%$//)
{
  $project_name_column = $lines[7];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^  kid\s*$/, 'child indented and without parent name');

if ($lines[8] =~ s/\s+\d+\s+-\s+\d+%$//)
{
  $project_name_column = $lines[8];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^existingParent\s*$/, 'existing parent not indented and has priority columns');

if ($lines[9] =~ s/\s+\d+\s+-\s+\d+%$//)
{
  $project_name_column = $lines[9];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^  child\s*$/, 'child of existing parent indented and without parent name');

if ($lines[10] =~ s/\s+\d+\s+-\s+\d+%$//)
{
  $project_name_column = $lines[10];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^myProject\.\s*$/, '\'myProject.\' not indented');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key bug.1056);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'bug.1056', 'Cleanup');

exit 0;

