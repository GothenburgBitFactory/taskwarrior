#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

import os
import re
import sys
import unittest
from time import time
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestTimesheet(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        # Create some tasks that were started/finished in different weeks, then verify
        # the presence and grouping in the timesheet report.
        #   P0   pending, this week
        #   PS0  started, this week
        #   PS1  started, last week
        #   PS2  started, 2wks ago
        #   D0   deleted, this week
        #   D1   deleted, last week
        #   D2   deleted, 2wks ago
        #   C0   completed, this week
        #   C1   completed, last week
        #   C2   completed, 2wks ago
        now      = int(time())
        seven    = now -  7 * 86400
        fourteen = now - 14 * 86400

        cls.t("add P0 entry:{0}".format(fourteen))
        cls.t("add PS0 entry:{0} start:{1} due:{2}".format(fourteen, now, now))
        cls.t("add PS1 entry:{0} start:{1}".format(fourteen, seven))
        cls.t("add PS2 entry:{0} start:{0}".format(fourteen))
        cls.t("add D0 entry:{0} end:{1} due:{2}".format(fourteen, now, now))
        cls.t("add D1 entry:{0} end:{1}".format(fourteen, seven))
        cls.t("add D2 entry:{0} end:{0}".format(fourteen))
        cls.t("/D[0-2]/ delete", input="all\n")
        cls.t("log C0 entry:{0} end:{1} due:{2}".format(fourteen, now, now))
        cls.t("log C1 entry:{0} end:{1}".format(fourteen, seven))
        cls.t("log C2 entry:{0} end:{0}".format(fourteen))

    def test_one_week(self):
        """One week of started and completed"""
        code, out, err = self.t("timesheet")

        expected = re.compile("Completed.+C0.+Started.+PS0", re.DOTALL)
        self.assertRegexpMatches(out, expected)

    def test_two_weeks(self):
        """Two weeks of started and completed"""
        code, out, err = self.t("timesheet 2")

        expected = re.compile(
            "Completed.+C0.+Started.+PS0.+"
            "Completed.+C1.+Started.+PS1", re.DOTALL)
        self.assertRegexpMatches(out, expected)

    def test_three_weeks(self):
        """Three weeks of started and completed"""
        code, out, err = self.t("timesheet 3")

        expected = re.compile(
            "Completed.+C0.+Started.+PS0.+"
            "Completed.+C1.+Started.+PS1.+"
            "Completed.+C2.+Started.+PS2", re.DOTALL)
        self.assertRegexpMatches(out, expected)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
