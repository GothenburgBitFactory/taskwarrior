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

import sys
import os
import unittest
import math
# Ensure python finds the local simpletap and basetest modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestCountdown(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t("add one      due:-1.2years")
        cls.t("add two      due:-9months")
        cls.t("add three    due:-3weeks")
        cls.t("add four     due:-7days")
        cls.t("add five     due:-1day")
        cls.t("add six      due:-12hours")
        cls.t("add seven    due:-1hour")
        cls.t("add eight    due:-30seconds")
        cls.t("add nine     due:1hour")
        cls.t("add ten      due:12hours")
        cls.t("add eleven   due:1days")
        cls.t("add twelve   due:7days")
        cls.t("add thirteen due:3weeks")
        cls.t("add fourteen due:9months")
        cls.t("add fifteen  due:1.2years")

    def setUp(self):
        """Executed before each test in the class"""

    def test_countdown_up(self):
        """Verify countdown sorting: ascending"""
        self.t.config("report.up.description",    "countdown+ report")
        self.t.config("report.up.columns",        "id,due.countdown,description")
        self.t.config("report.up.labels",         "ID,Countdown,Description")
        self.t.config("report.up.filter",         "status:pending")
        self.t.config("report.up.sort",           "due+")

        code, out, err = self.t("up")
        self.assertRegexpMatches(out, " one\n.+ two\n")
        self.assertRegexpMatches(out, " two\n.+ three\n")
        self.assertRegexpMatches(out, " three\n.+ four\n")
        self.assertRegexpMatches(out, " four\n.+ five\n")
        self.assertRegexpMatches(out, " five\n.+ six\n")
        self.assertRegexpMatches(out, " six\n.+ seven\n")
        self.assertRegexpMatches(out, " seven\n.+ eight\n")
        self.assertRegexpMatches(out, " eight\n.+ nine\n")
        self.assertRegexpMatches(out, " nine\n.+ ten\n")
        self.assertRegexpMatches(out, " ten\n.+ eleven\n")
        self.assertRegexpMatches(out, " eleven\n.+ twelve\n")
        self.assertRegexpMatches(out, " twelve\n.+ thirteen\n")
        self.assertRegexpMatches(out, " thirteen\n.+ fourteen\n")
        self.assertRegexpMatches(out, " fourteen\n.+ fifteen\n")

    def test_countdown_down(self):
        """Verify countdown sorting: descending"""
        self.t.config("report.down.description",  "countdown- report")
        self.t.config("report.down.columns",      "id,due.countdown,description")
        self.t.config("report.down.labels",       "ID,Countdown,Description")
        self.t.config("report.down.filter",       "status:pending")
        self.t.config("report.down.sort",         "due-")

        code, out, err = self.t("down")
        self.assertRegexpMatches(out, " fifteen\n.+ fourteen\n")
        self.assertRegexpMatches(out, " fourteen\n.+ thirteen\n")
        self.assertRegexpMatches(out, " thirteen\n.+ twelve\n")
        self.assertRegexpMatches(out, " twelve\n.+ eleven\n")
        self.assertRegexpMatches(out, " eleven\n.+ ten\n")
        self.assertRegexpMatches(out, " ten\n.+ nine\n")
        self.assertRegexpMatches(out, " nine\n.+ eight\n")
        self.assertRegexpMatches(out, " eight\n.+ seven\n")
        self.assertRegexpMatches(out, " seven\n.+ six\n")
        self.assertRegexpMatches(out, " six\n.+ five\n")
        self.assertRegexpMatches(out, " five\n.+ four\n")
        self.assertRegexpMatches(out, " four\n.+ three\n")
        self.assertRegexpMatches(out, " three\n.+ two\n")
        self.assertRegexpMatches(out, " two\n.+ one\n")


class TestFormatDepends(TestCase):
    def setUp(self):
        self.t = Task()
        self.t("add zero")
        self.t("add one depends:1")

    def test_depends_default(self):
        self.t.config("report.formatdep.columns", "description,depends")
        code, out, err = self.t("formatdep")
        self.assertRegexpMatches(out, "one\s+1")

    def test_depends_count(self):
        self.t.config("report.formatdep.columns", "description,depends.count")
        code, out, err = self.t("formatdep")
        self.assertRegexpMatches(out, "one\s+\[1\]")


class TestBug101(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

        # Define report with truncated_count style
        self.t.config("report.bug101.columns", "description.truncated_count")

        # Find screen width in order to generate long enough string
        code, out, err = self.t("_get context.width")
        self.width = int(out)

        # Since task strips leading and trailing spaces, for the purposes
        # of these tests, ensure description contains no spaces so we know
        # exactly what string we are expecting
        self.short_description = "A_task_description_"

        # Generate long string
        self.long_description = self.short_description * int(math.ceil(float(self.width)/len(self.short_description)))

    def test_short_no_count(self):
        """101: Check short description with no annotations"""
        self.t(("add", self.short_description))
        code, out, err = self.t("bug101")
        expected = self.short_description
        self.assertIn(expected, out)

    def test_short_with_count(self):
        """101: Check short description with annotations"""
        self.t(("add", self.short_description))
        self.t("1 annotate 'A task annotation'")
        code, out, err = self.t("bug101")
        expected = self.short_description + " [1]"
        self.assertIn(expected, out)

    def test_long_no_count(self):
        """101: Check long description with no annotations"""
        self.t(("add", self.long_description))
        code, out, err = self.t("bug101")
        expected = self.long_description[:(self.width - 3)] + "..."
        self.assertIn(expected, out)

    def test_long_with_count(self):
        """101: Check long description with annotations"""
        self.t(("add", self.long_description))
        self.t("1 annotate 'A task annotation'")
        code, out, err = self.t("bug101")
        expected = self.long_description[:(self.width - 7)] + "... [1]"
        self.assertIn(expected, out)

    def test_long_with_double_digit_count(self):
        """101: Check long description with double digit amount of annotations"""
        self.t(("add", self.long_description))
        for i in range(10):
            self.t("1 annotate 'A task annotation'")
        code, out, err = self.t("bug101")
        expected = self.long_description[:(self.width - 8)] + "... [10]"
        self.assertIn(expected, out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
