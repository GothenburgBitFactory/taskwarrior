#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
use JSON;

# Use the taskwarrior 1.9.4+ _query command to issue a query and return JSON
my $command = '/usr/local/bin/task _query ' . join (' ', @ARGV);

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

