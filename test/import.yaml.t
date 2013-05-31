#! /usr/bin/env perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 16;

my $source_dir = $0;
$source_dir =~ s{[^/]+$}{..};

# Create the rc file.
if (open my $fh, '>', 'import.rc')
{
  print $fh "data.location=.\n",
            "dateformat=m/d/Y\n";
  close $fh;
  ok (-r 'import.rc', 'Created import.rc');
}

# Create import file.
if (open my $fh, '>', 'import.txt')
{
  print $fh <<EOF;
%YAML 1.1
---
  task:
    uuid: 00000000-0000-0000-0000-000000000000
    description: zero
    project: A
    status: pending
    entry: 20110901T120000Z
  task:
    uuid: 11111111-1111-1111-1111-111111111111
    description: one
    project: B
    status: pending
    entry: 20110902T120000Z
  task:
    uuid: 22222222-2222-2222-2222-222222222222
    description: two
    status: completed
    entry: 20110903T010000Z
    end: 20110904T120000Z
...
EOF

  close $fh;
  ok (-r 'import.txt', 'Created sample import data');
}

# Convert YAML --> task JSON.
qx{$source_dir/scripts/add-ons/import-yaml.pl <import.txt >import.json 2>&1};

# Import the JSON.
my $output = qx{../src/task rc:import.rc import import.json 2>&1 >/dev/null};
like ($output, qr/Imported 3 tasks\./, '3 tasks imported');

$output = qx{../src/task rc:import.rc list 2>&1};
# ID Project Age     Description
# -- ------- ------- -----------
#  1 A       1.5 yrs zero
#  2 B       1.5 yrs one
# 
# 2 tasks

like   ($output, qr/1.+A.+zero/, 't1 present');
like   ($output, qr/2.+B.+one/,  't2 present');
unlike ($output, qr/3.+two/,     't3 missing');

$output = qx{../src/task rc:import.rc completed 2>&1};
# Complete  Age     Description
# --------- ------- -----------
# 2/13/2009 1.5 yrs two
# 
# 1 task

unlike ($output, qr/1.+A.+zero/,      't1 missing');
unlike ($output, qr/2.+B.+one/,       't2 missing');
like   ($output, qr/9\/4\/2011.+two/, 't3 present');

# Make sure that a duplicate task cannot be imported.
$output = qx{../src/task rc:import.rc import import.json 2>&1 >/dev/null};
like ($output, qr/Cannot add task because the uuid '.{36}' is not unique\./, 'error on duplicate uuid');

# Create import file.
if (open my $fh, '>', 'import.txt')
{
  print $fh <<EOF;
%YAML 1.1
---
task:
  uuid: 44444444-4444-4444-4444-444444444444
  description: three
  status: pending
  entry: 1234567889
...
EOF

  close $fh;
  ok (-r 'import.txt', 'Created second sample import data');
}

# Convert YAML --> task JSON.
qx{$source_dir/scripts/add-ons/import-yaml.pl <import.txt >import.json 2>&1};

# Import the JSON.
$output = qx{../src/task rc:import.rc import import.json 2>&1 >/dev/null};
like ($output, qr/Imported 1 tasks\./, '1 task imported');

# Verify.
$output = qx{../src/task rc:import.rc list 2>&1};
# ID Project Pri Due Active Age  Description
# -- ------- --- --- ------ ---- -----------
#  3                        2.6y three
#  1 A                        3d zero
#  2 B                        2d one
like ($output, qr/1.+A.+zero/, 't1 present');
like ($output, qr/2.+B.+one/,  't2 present');
like ($output, qr/3.+three/,   't3 present');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data import.rc import.txt import.json);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'import.rc'      &&
    ! -r 'import.txt'     &&
    ! -r 'import.json', 'Cleanup');
exit 0;

