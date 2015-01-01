#! /usr/bin/python
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

import sys
import commands
import json

# Use the taskwarrior 2.0+ export command to filter and return JSON
command = "/usr/local/bin/task rc.verbose=nothing rc.json.array=no export " + " ".join (sys.argv[1:])

# Generate output.
print "<tasks>"
for task in commands.getoutput (command).split (",\n"):
    data = json.loads (task)
    print "  <task>"
    for name,value in data.items ():
        if name == "annotations":
            print "    <annotations>"
            for anno in value:
                print "      <annotation>"
                for name,value in anno.items ():
                    print "        <{0}>{1}</{0}>".format (name, value)
                    print "      </annotation>"
                    print "    </annotations>"
        elif name == "tags":
            print "    <tags>"
            for tag in value:
                print "      <tag>{0}</tag>".format (tag)
                print "    </tags>"
        else:
            print "    <{0}>{1}</{0}>".format (name, value)
    print "  </task>"
print "</tasks>"
sys.exit (0)

################################################################################

