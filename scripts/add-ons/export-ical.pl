#! /usr/bin/perl
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

# Give a nice error if the (non-standard) JSON module is not installed.
eval "use JSON";
if ($@)
{
  print "Error: You need to install the JSON Perl module.\n";
  exit 1;
}

# Use the taskwarrior 2.0+ export command to filter and return JSON
my $command = join (' ', ("env PATH=$ENV{PATH} task rc.verbose=nothing rc.json.array=no export", @ARGV));
if ($command =~ /No matches/)
{
  printf STDERR $command;
  exit 1;
}

# Generate output.
print "BEGIN:VCALENDAR\n",
      "VERSION:2.0\n",
      "PRODID:=//GBF/taskwarrior 1.9.4//EN\n";

for my $task (split /,$/ms, qx{$command})
{
  my $data = from_json ($task);

  print "BEGIN:VTODO\n";
  print "UID:$data->{'uuid'}\n";
  print "DTSTAMP:$data->{'entry'}\n";
  print "DTSTART:$data->{'start'}\n" if exists $data->{'start'};
  print "DUE:$data->{'due'}\n"       if exists $data->{'due'};
  print "COMPLETED:$data->{'end'}\n" if exists $data->{'end'};
  print "SUMMARY:$data->{'description'}\n";
  print "CLASS:PRIVATE\n";
  print "CATEGORIES:", join (',', @{$data->{'tags'}}), "\n" if exists $data->{'tags'};

  # Priorities map to a 1-9 scale.
  if (exists $data->{'priority'})
  {
    print "PRIORITY:", ($data->{'priority'} eq 'H' ? '1' :
                        $data->{'priority'} eq 'M' ? '5' :
                                                     '9'), "\n";
  }

  # Status maps differently.
  my $status = $data->{'status'};
  if ($status eq 'pending' || $status eq 'waiting')
  {
    print "STATUS:", (exists $data->{'start'} ? 'IN-PROCESS' : 'NEEDS-ACTION'), "\n";
  }
  elsif ($status eq 'completed')
  {
    print "STATUS:COMPLETED\n";
  }
  elsif ($status eq 'deleted')
  {
    print "STATUS:CANCELLED\n";
  }

  # Annotations become comments.
  if (exists $data->{'annotations'})
  {
    print "COMMENT:$_->{'description'}\n" for @{$data->{'annotations'}};
  }

  print "END:VTODO\n";
}

print "END:VCALENDAR\n";
exit 0;

################################################################################

