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

import re

class TAP:
  """TAP-compliant unit test class."""

  def __init__(self, planned = 0):
    self.planned = planned
    self.counter = 0
    self.passed = 0
    self.failed = 0
    self.skipped = 0
    print "1..%d " % self.planned

  def __del__(self):
    percentPassed = 0.0 
    if self.planned > 0:
      percentPassed = (100.0 * self.passed) / max (self.planned, self.passed + self.failed + self.skipped)

    if self.counter < self.planned:
      print "# Only %d tests, out of a planned %d were run." % (self.counter, self.planned)
      self.skipped += self.planned - self.counter
    elif self.counter > self.planned:
      print "# %d tests were run, but only %d were planned." % (self.counter, self.planned)

    print "# %d passed, %d failed, %d skipped. %.2f%% passed." % (self.passed, self.failed, self.skipped, percentPassed)

  def plan(self, planned):
    self.planned = planned
    print "1..%d " % self.planned

  def planMore(self, extra):
    self.planned += extra
    print "1..%d" % self.planned

  def ok(self, expression, description):
    self.counter += 1
    if bool(expression):
      self.passed += 1
      print "ok %d - %s" % (self.counter, description)
    else:
      self.failed += 1
      print "not ok %d - %s" % (self.counter, description)

  def notok(self, expression, description):
    self.counter += 1
    if not bool(expression):
      self.passed += 1
      print "ok %d - %s" % (self.counter, description)
    else:
      self.failed += 1
      print "not ok %d - %s" % (self.counter, description)

  def equals(self, actual, expected, description):
    self.counter += 1
    if actual == expected:
      self.passed += 1
      print "ok %d - %s" % (self.counter, description)
    else:
      self.failed += 1
      print "not ok %d - %s" % (self.counter, description)
      print "# expected:", expected, "\n#      got:", actual

  def like(self, actual, pattern, description):
    self.counter += 1
    if re.search(pattern, actual):
      self.passed += 1
      print "ok %d - %s" % (self.counter, description)
    else:
      self.failed += 1
      print "not ok %d - %s" % (self.counter, description)

  def unlike(self, actual, pattern, description):
    self.counter += 1
    if re.search(pattern, actual):
      self.failed += 1
      print "not ok %d - %s" % (self.counter, description)
    else:
      self.passed += 1
      print "ok %d - %s" % (self.counter, description)

  def diag(self, stuff):
    for line in stuff.strip().split("\n"):
      print "#", line.strip()

  def skip(self, message):
    self.counter += 1
    self.skipped += 1
    print "skip %d %s" % (self.counter, message)

  def passed(self, message):
    self.counter += 1
    self.passed += 1
    print "ok %d %s" % (self.counter, message)

  def fail(self, message):
    self.counter += 1
    self.failed += 1
    print "not ok %d %s" % (self.counter, message)

  def skip(self, message):
    self.counter += 1
    self.skipped += 1
    print "skip %d %s" % (self.counter, message)

################################################################################
