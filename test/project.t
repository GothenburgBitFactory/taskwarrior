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
use Test::More tests => 18;

# Create the rc file.
if (open my $fh, '>', 'pro.rc')
{
  print $fh "data.location=.\n",
            "confirmation=off\n";
  close $fh;
  ok (-r 'pro.rc', 'Created pro.rc');
}

# Test the project status numbers.
my $output = qx{../src/task rc:pro.rc add one pro:foo 2>&1 >/dev/null};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 0% complete \(1 of 1 tasks remaining\)\./, 'add one');

$output = qx{../src/task rc:pro.rc add two pro:'foo' 2>&1 >/dev/null};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 0% complete \(2 of 2 tasks remaining\)\./, 'add two');

$output = qx{../src/task rc:pro.rc add three pro:'foo' 2>&1 >/dev/null};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 0% complete \(3 of 3 tasks remaining\)\./, 'add three');

$output = qx{../src/task rc:pro.rc add four pro:'foo' 2>&1 >/dev/null};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 0% complete \(4 of 4 tasks remaining\)\./, 'add four');

$output = qx{../src/task rc:pro.rc 1 done 2>&1 >/dev/null};
like ($output, qr/Project 'foo' is 25% complete \(3 of 4 tasks remaining\)\./, 'done one');

$output = qx{../src/task rc:pro.rc 2 delete 2>&1 >/dev/null};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 33% complete \(2 of 3 tasks remaining\)\./, 'delete two');

$output = qx{../src/task rc:pro.rc 3 modify pro:bar 2>&1 >/dev/null};
like ($output, qr/The project 'foo' has changed\.  Project 'foo' is 50% complete \(1 of 2 tasks remaining\)\./, 'change project');
like ($output, qr/The project 'bar' has changed\.  Project 'bar' is 0% complete \(1 of 1 tasks remaining\)\./, 'change project');

# Test projects with spaces in them.
$output = qx{../src/task rc:pro.rc 3 modify pro:\\"foo bar\\" 2>&1 >/dev/null};
like ($output, qr/The project 'foo bar' has changed\./, 'project with spaces');

# Bug 1056: Project indentation.
# see also the tests of helper functions for CmdProjects in util.t.cpp
qx{../src/task rc:pro.rc add testing project:existingParent 2>&1 >/dev/null};
qx{../src/task rc:pro.rc add testing project:existingParent.child 2>&1 >/dev/null};
qx{../src/task rc:pro.rc add testing project:abstractParent.kid 2>&1 >/dev/null};
qx{../src/task rc:pro.rc add testing project:.myProject 2>&1 >/dev/null};
qx{../src/task rc:pro.rc add testing project:myProject. 2>&1 >/dev/null};
qx{../src/task rc:pro.rc add testing project:.myProject. 2>&1 >/dev/null};

$output = qx{../src/task rc:pro.rc projects 2>&1};
my @lines = split ('\n',$output);

# It's easier to make a pattern for the end than the beginning because priority
# counts are more predictable than project names.
my $project_name_column;
if ($lines[4] =~ s/\d+\s+\d+\s+\d+\s+\d+\s+\d+$//)
{
  $project_name_column = $lines[4];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^\.myProject\s*$/, '\'.myProject\' not indented');

if ($lines[5] =~ s/\d+\s+\d+\s+\d+\s+\d+\s+\d+$//)
{
  $project_name_column = $lines[5];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^\.myProject\.\s*$/, '\'.myProject.\' not indented');

if ($lines[6] =~ s/\d+\s+\d+\s+\d+\s+\d+\s+\d+$//)
{
  $project_name_column = "error";
}
else
{
  $project_name_column = $lines[6];
}
like ($project_name_column, qr/^abstractParent\s*$/, 'abstract parent not indented and no priority columns');

if ($lines[7] =~ s/\d+\s+\d+\s+\d+\s+\d+\s+\d+$//)
{
  $project_name_column = $lines[7];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^  kid\s*$/, 'child indented and without parent name');

if ($lines[8] =~ s/\d+\s+\d+\s+\d+\s+\d+\s+\d+$//)
{
  $project_name_column = $lines[8];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^existingParent\s*$/, 'existing parent not indented and has priority columns');

if ($lines[9] =~ s/\d+\s+\d+\s+\d+\s+\d+\s+\d+$//)
{
  $project_name_column = $lines[9];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^  child\s*$/, 'child of existing parent indented and without parent name');

if ($lines[12] =~ s/\d+\s+\d+\s+\d+\s+\d+\s+\d+$//)
{
  $project_name_column = $lines[12];
}
else
{
  $project_name_column = "error";
}
like ($project_name_column, qr/^myProject\.\s*$/, '\'myProject.\' not indented');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data pro.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'pro.rc', 'Cleanup');

exit 0;

