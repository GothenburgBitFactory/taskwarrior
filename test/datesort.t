#!/usr/bin/env python3
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
# https://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestDateSort(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_datesort(self):
        """Verify dates sort properly with a report date format that hides date details"""
        self.t.config("verbose",                       "nothing")
        self.t.config("dateformat",                    "YMD")
        self.t.config("report.small_list.columns",     "due,description")
        self.t.config("report.small_list.labels",      "Due,Description")
        self.t.config("report.small_list.sort",        "due+")
        self.t.config("report.small_list.filter",      "status:pending")
        self.t.config("report.small_list.dateformat",  "D")

        self.t("add one due:20150101")
        self.t("add two due:20150201")
        self.t("add three due:20150301")

        # The sort order shoudl be correct, despite the display format being
        # identical for all tasks. This proves sorting is correct, independent
        # of display.
        code, out, err = self.t("small_list")
        self.assertEqual("01  one\n01  two\n01  three\n", out)

        code, out, err = self.t("rc.report.small_list.sort=due- small_list")
        self.assertEqual("01  three\n01  two\n01  one\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
