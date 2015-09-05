#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# http://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBugAnnual(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_annual_creep(self):
        """Verify 'annual' recurring tasks don't creep"""
        self.t.config("dateformat",            "YMD")
        self.t.config("report.annual.labels",  "ID,Due")
        self.t.config("report.annual.columns", "id,due")
        self.t.config("report.annual.filter",  "status:pending")
        self.t.config("report.annual.sort",    "due+")

        # If a task is added with a due date ten years ago, with an annual recurrence,
        # then the synthetic tasks in between then and now have a due date that creeps.
        #
        # ID Due        Description
        # -- ---------- -----------
        #  4 1/1/2002   foo
        #  5 1/1/2003   foo
        #  6 1/1/2004   foo
        #  7 1/1/2005   foo
        #  8 1/1/2006   foo
        #  9 1/1/2007   foo
        # 10 1/1/2008   foo
        # 11 1/1/2009   foo
        # 12 1/1/2010   foo
        #  2 1/1/2000   foo
        #  3 1/1/2001   foo

        self.t("add foo due:20000101 recur:annual until:20150101")
        code, out, err = self.t("annual")
        self.assertIn(" 2 20000101", out)
        self.assertIn(" 3 20010101", out)
        self.assertIn(" 4 20020101", out)
        self.assertIn(" 5 20030101", out)
        self.assertIn(" 6 20040101", out)
        self.assertIn(" 7 20050101", out)
        self.assertIn(" 8 20060101", out)
        self.assertIn(" 9 20070101", out)
        self.assertIn("10 20080101", out)
        self.assertIn("11 20090101", out)
        self.assertIn("12 20100101", out)
        self.assertIn("13 20110101", out)
        self.assertIn("14 20120101", out)
        self.assertIn("15 20130101", out)
        self.assertIn("16 20140101", out)
        self.assertIn("17 20150101", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
