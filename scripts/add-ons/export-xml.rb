#! /usr/bin/ruby
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

require 'rubygems'
require 'json'

# Use the taskwarrior 2.0+ export command to filter and return JSON
lines = IO.popen("/usr/local/bin/task rc.verbose=nothing rc.json.array=no export " + ARGV.join(" ")).readlines

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

