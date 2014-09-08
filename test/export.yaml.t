#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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

my $source_dir = $0;
$source_dir =~ s{[^/]+$}{..};

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n",
            "verbose=no\n";
  close $fh;
}

# Add two tasks, export, examine result.
# TODO Add annotations.
qx{../src/task rc:$rc add priority:H project:A one 2>&1};
qx{../src/task rc:$rc add +tag1 +tag2 two 2>&1};

my $output = qx{../src/task rc:$rc export | $source_dir/scripts/add-ons/export-yaml.pl > ./export.txt 2>&1};
my @lines;
if (open my $fh, '<', './export.txt')
{
  @lines = <$fh>;
  close $fh;
}

like ($lines[0],  qr/^\%YAML 1\.1$/,                "$ut: export YAML line 1");
like ($lines[1],  qr/^---$/,                        "$ut: export YAML line 2");
like ($lines[2],  qr/^  task:$/,                    "$ut: export YAML line 3");
like ($lines[3],  qr/^    description: one$/,       "$ut: export YAML line 4");
like ($lines[4],  qr/^    entry: \d{8}T\d{6}Z$/,    "$ut: export YAML line 5");
like ($lines[5],  qr/^    id: \d+$/,                "$ut: export YAML line 6");
like ($lines[6],  qr/^    modified: \d{8}T\d{6}Z$/, "$ut: export YAML line 7");
like ($lines[7],  qr/^    priority: H$/,            "$ut: export YAML line 8");
like ($lines[8],  qr/^    project: A$/,             "$ut: export YAML line 9");
like ($lines[9],  qr/^    status: pending$/,        "$ut: export YAML line 10");
like ($lines[10], qr/^    urgency: .+$/,            "$ut: export YAML line 11");
like ($lines[11], qr/^    uuid: .+$/,               "$ut: export YAML line 12");
like ($lines[12], qr/^  task:$/,                    "$ut: export YAML line 13");
like ($lines[13], qr/^    description: two$/,       "$ut: export YAML line 14");
like ($lines[14], qr/^    entry: \d{8}T\d{6}Z$/,    "$ut: export YAML line 15");
like ($lines[15], qr/^    id: \d+$/,                "$ut: export YAML line 16");
like ($lines[16], qr/^    modified: \d{8}T\d{6}Z$/, "$ut: export YAML line 17");
like ($lines[17], qr/^    status: pending$/,        "$ut: export YAML line 18");
like ($lines[18], qr/^    tags: tag1,tag2$/,        "$ut: export YAML line 19");
like ($lines[19], qr/^    urgency: .+$/,            "$ut: export YAML line 20");
like ($lines[20], qr/^    uuid: .+$/,               "$ut: export YAML line 21");
like ($lines[21], qr/^\.\.\.$/,                     "$ut: export YAML line 22");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data export.txt), $rc;
exit 0;

