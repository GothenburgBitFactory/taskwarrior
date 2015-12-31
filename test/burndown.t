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
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBurndownCommand(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t("add one entry:-2y")
        cls.t("add two entry:-1y")
        cls.t("2 start")
        cls.t("add three entry:soy")
        cls.t("3 delete", input="y\n")
        cls.t("add four")
        cls.t("4 start")
        cls.t("4 done")
        cls.t("add five")
        cls.t("5 start")
        cls.t("5 done")
        cls.t("log six")

    def setUp(self):
        """Executed before each test in the class"""

    def test_burndown_daily(self):
        """Ensure burndown.daily generates a chart"""
        code, out, err = self.t("burndown.daily")
        self.assertIn("Daily Burndown ()", out)
        self.assertIn(".", out)
        self.assertIn("+", out)
        self.assertIn("X", out)

    def test_burndown_daily_color(self):
        """Ensure burndown.daily with color, generates a chart"""
        code, out, err = self.t("burndown.daily rc._forcecolor:on")
        self.assertIn("Daily Burndown ()", out)
        self.assertNotIn("X", out)

    def test_burndown_weekly(self):
        """Ensure burndown.weekly generates a chart"""
        code, out, err = self.t("burndown.weekly")
        self.assertIn("Weekly Burndown ()", out)
        self.assertIn(".", out)
        self.assertIn("+", out)
        self.assertIn("X", out)

    def test_burndown_monthly(self):
        """Ensure burndown.monthly generates a chart"""
        code, out, err = self.t("burndown.monthly")
        self.assertIn("Monthly Burndown ()", out)
        self.assertIn(".", out)
        self.assertIn("+", out)
        self.assertIn("X", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
