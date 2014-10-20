#! /usr/bin/perl
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
    if ($tags ne '')
    {
      $tags =~ s/,/","/g;
      $json .= ",\"tags\":[\"${tags}\"]";
    }
    if (%annotations)
    {
      $json .= ",\"annotations\":[";
      my $pre = "";
      for my $key (sort keys %annotations) {
        $json .= "${pre}{\"entry\":\"${key}\",\"description\":\"${annotations{$key}}\"}";
        $pre = ",";
      }
      $json .= "]";
    }
    $json .= ",\"description\":\"${description}\"}";

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

        $annotations{$anno_entry} = $anno_desc;
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

print "[\n", join ("\n", @tasks), "\n]\n";
exit 0;

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

