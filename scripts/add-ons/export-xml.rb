#! /usr/bin/ruby
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

require 'rubygems'
require 'json'

# Use the taskwarrior 1.9.4+ _query command to issue a query and return JSON
lines = IO.popen("/usr/local/bin/task _query " + ARGV.join(" ")).readlines

# Generate output.
print "<tasks>\n"
lines.each do |line|
  data = JSON.parse(line)
  print "  <task>\n"
  data.each do |key,value|
    if key == "annotations"
      print "    <annotations>\n"
      value.each do |anno|
        print "      <annotation>\n"
        anno.each do |key,value|
          print "        <#{key}>#{value}</#{key}>\n"
        end
        print "      </annotation>\n"
      end
      print "    </annotations>\n"
    elsif key == "tags"
      print "    <tags>\n"
      value.each do |tag|
        print "      <tag>#{tag}</tag>\n"
      end
      print "    </tags>\n"
    else
      print "    <#{key}>#{value}</#{key}>\n"
    end
  end
  print "  </task>\n"
end
print "</tasks>\n"
exit 0

################################################################################

