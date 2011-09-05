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
use Time::Local;

my @tasks;

my $status = '';
my $uuid = '';
my $priority = '';
my $entry = '';
my $end = '';
my $project = '';
my $tags = '';
my $description = '';
my $due = '';

my %annotations;
my $mid_anno = 0;
my $anno_entry = '';
my $anno_desc = '';

while (my $yaml = <>)
{
  chomp $yaml;
  next if $yaml =~ /\sid:/;
  next if $yaml =~ /\sannotations:/;

  if ($yaml =~ /task:|\.\.\./ && $description ne '')
  {
    # Compose the JSON
    my $json  = "{\"status\":\"${status}\"";
    $json .= ",\"uuid\":\"${uuid}\""             if $uuid ne '';
    $json .= ",\"priority\":\"${priority}\""     if $priority ne '';
    $json .= ",\"project\":\"${project}\""       if $project ne '';
    $json .= ",\"entry\":\"${entry}\""           if $entry ne '';
    $json .= ",\"end\":\"${end}\""               if $end ne '';
    $json .= ",\"due\":\"${due}\""               if $due ne '';
    $json .= ",\"tags\":\"${tags}\""             if $tags ne '';
    $json .= ",\"description\":\"${description}\"}";

    for my $key (sort keys %annotations)
    {
      $json .= ",\"${key}\":\"" . $annotations{$key} . "\""
    }

    push @tasks, $json;

    $status = $uuid = $priority = $entry = $end = $project = $tags =
      $description = $due = $anno_entry = $anno_desc = '';
    $mid_anno = 0;
    %annotations = ();
    $yaml = '';
  }
  else
  {
    $mid_anno = 1 if $yaml =~ /annotation:/;
    $anno_entry  = $1 if $mid_anno && $yaml =~ /entry:\s*(\S+)/;
    if ($mid_anno)
    {
      if ($yaml =~ /description:\s*(.+)/)
      {
        $anno_desc = $1;
        $mid_anno = 0;

        $annotations{'annotation_' . epoch ($anno_entry)} = $anno_desc;
      }
    }
    else
    {
      $entry       = $1 if $yaml =~ /entry:\s*(\S+)/;
      $description = $1 if $yaml =~ /description:\s*(.+)/;
    }

    $status      = $1 if $yaml =~ /status:\s*(\S+)/;
    $uuid        = $1 if $yaml =~ /uuid:\s*(\S+)/;
    $priority    = $1 if $yaml =~ /priority:\s*(\S+)/;
    $project     = $1 if $yaml =~ /project:\s*(\S+)/;
    $end         = $1 if $yaml =~ /end:\s*(\S+)/;
    $due         = $1 if $yaml =~ /due:\s*(\S+)/;
    $tags        = $1 if $yaml =~ /tags:\s*(\S+)/;
  }
}

print "[\n", join (",\n", @tasks), "\n]\n";
exit 0;

################################################################################
sub epoch
{
  my ($input) = @_;

  my ($Y, $M, $D, $h, $m, $s) = $input =~ /(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})(\d{2})Z/;

  return timegm ($s, $m, $h, $D, $M-1, $Y-1900);
}

################################################################################

__DATA__
%YAML 1.1
---
  task:
    annotations:
      annotation:
        entry: 20100706T025311Z
        description: Also needs to ignore control codes
    description: text.cpp/extractLines needs to calculate string length in a way that supports UTF8
    entry: 20090319T151633Z
    id: 9
    project: task-2.0
    status: pending
    tags: bug,utf8,next
    uuid: 0c4cf066-9413-4862-9dc8-0793f158a649
...

