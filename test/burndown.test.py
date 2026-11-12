#!/usr/bin/env python3
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
        self.assertIn("Daily Burndown", out)
        self.assertIn(".", out)
        self.assertIn("+", out)
        self.assertIn("X", out)

    def test_burndown_daily_non_cumulative(self):
        """Ensure burndown.daily in non-cumulative mode generates a chart"""
        self.t.config("burndown.cumulative", "0")
        code, out, err = self.t("burndown.daily")
        self.assertIn("Daily Burndown", out)
        self.assertIn(".", out)
        self.assertIn("+", out)
        self.assertIn("X", out)

    def test_burndown_daily_color(self):
        """Ensure burndown.daily with color, generates a chart"""
        code, out, err = self.t("burndown.daily rc._forcecolor:on")
        self.assertIn("Daily Burndown", out)
        self.assertNotIn("X", out)

    def test_burndown_weekly(self):
        """Ensure burndown.weekly generates a chart"""
        code, out, err = self.t("burndown.weekly")
        self.assertIn("Weekly Burndown", out)
        self.assertIn(".", out)
        self.assertIn("+", out)
        self.assertIn("X", out)

    def test_burndown_monthly(self):
        """Ensure burndown.monthly generates a chart"""
        code, out, err = self.t("burndown.monthly")
        self.assertIn("Monthly Burndown", out)
        self.assertIn(".", out)
        self.assertIn("+", out)
        self.assertIn("X", out)

    def test_burndown_annual(self):
        """Ensure burndown.annual generates a chart"""
        code, out, err = self.t("burndown.annual")
        self.assertIn("Annual Burndown", out)
        self.assertIn(".", out)
        self.assertIn("+", out)
        self.assertIn("X", out)

    def test_burndown_oversized_width(self):
        """Reject oversized charts before generating out-of-range dates."""
        for width in (0, 65536):
            for period in ("daily", "weekly", "monthly", "annual"):
                with self.subTest(width=width, period=period):
                    code, out, err = self.t(
                        "burndown.{} rc.defaultwidth:{} rc.detection:off".format(
                            period, width
                        )
                    )
                    self.assertEqual(code, 0)
                    self.assertIn("Terminal window too large to draw a graph.", out)

    def test_burndown_invalid_height(self):
        """Reject invalid heights before generating out-of-range dates."""
        task = Task()
        task.faketime("2026-11-12 12:00:00")
        task("add dimension test entry:2026-01-01")
        for height, size in ((1, "small"), (0, "large"), (1009, "large")):
            for period in ("daily", "weekly", "monthly", "annual"):
                with self.subTest(height=height, period=period):
                    code, out, err = task(
                        "burndown.{} rc.defaultwidth:200 rc.defaultheight:{} "
                        "rc.reserved.lines:0 rc.detection:off".format(period, height)
                    )
                    self.assertEqual(code, 0)
                    self.assertIn(
                        "Terminal window too {} to draw a graph.".format(size), out
                    )

    def test_burndown_height_boundaries(self):
        """The minimum and maximum drawable graph heights still render."""
        for height in (13, 1008):
            for period in ("daily", "weekly", "monthly", "annual"):
                with self.subTest(height=height, period=period):
                    code, out, err = self.t(
                        "burndown.{} rc.defaultwidth:80 rc.defaultheight:{} "
                        "rc.reserved.lines:0 rc.detection:off".format(period, height)
                    )
                    self.assertEqual(code, 0)
                    self.assertIn("Burndown", out)
                    self.assertNotIn("Terminal window too", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
