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


class TestCalendarCommandLine(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_basic_command(self):
        """Verify 'calendar' does not fail"""
        code, out, err = self.t("calendar")
        self.assertIn("Su Mo Tu We Th Fr Sa", out)

    def test_basic_command_offset(self):
        """Verify 'calendar rc.calendar.offset:on rc.calendar.offset.value:1' does not fail"""
        code, out, err = self.t("calendar rc.calendar.offset:on rc.calendar.offset.value:1")
        self.assertIn("Su Mo Tu We Th Fr Sa", out)

    def test_basic_command(self):
        """Verify 'calendar rc.weekstart:Monday' does not fail'"""
        code, out, err = self.t("calendar rc.weekstart:Monday")
        self.assertIn("Mo Tu We Th Fr Sa Su", out)

    def test_basic_command_color(self):
        """Verify 'calendar rc._forcecolor:on' does not fail"""
        code, out, err = self.t("calendar rc._forcecolor:on")
        self.assertRegexpMatches(out, "Su.+Mo.+Tu.+We.+Th.+Fr.+Sa")

    def test_basic_command_details(self):
        """Verify 'calendar rc.calendar.details:full rc.calendar.details.report:list' does not fail"""
        self.t("add task_with_due_date due:tomorrow")
        code, out, err = self.t("calendar rc.calendar.details:full rc.calendar.details.report:list")
        self.assertIn("task_with_due_date", out)

    def test_basic_command_details_color(self):
        """Verify 'calendar rc.calendar.details:full rc.calendar.details.report:list rc._forcecolor:on' does not fail"""
        self.t("add task_with_due_date due:tomorrow")
        code, out, err = self.t("calendar rc.calendar.details:full rc.calendar.details.report:list rc._forcecolor:on")
        self.assertIn("task_with_due_date", out)

    def test_basic_command_holidays(self):
        """Verify 'calendar rc.calendar.holidays:full' does not fail"""
        code, out, err = self.t("calendar rc.calendar.holidays:full")
        self.assertIn("Date Holiday", out)

    def test_y_argument(self):
        """Verify 'calendar y' does not fail"""
        code, out, err = self.t("calendar y")
        self.assertIn("January",   out)
        self.assertIn("February",  out)
        self.assertIn("March",     out)
        self.assertIn("April",     out)
        self.assertIn("May",       out)
        self.assertIn("June",      out)
        self.assertIn("July",      out)
        self.assertIn("August",    out)
        self.assertIn("September", out)
        self.assertIn("October",   out)
        self.assertIn("November",  out)
        self.assertIn("December",  out)
        self.assertNotIn("Could not recognize argument", err)

    def test_due_argument(self):
        """Verify 'calendar due' does not fail"""
        code, out, err = self.t("calendar due")
        self.assertNotIn("Could not recognize argument", err)

    def test_2015_argument(self):
        """Verify 'calendar 2015' does not fail"""
        code, out, err = self.t("calendar 2015")
        self.assertNotIn("Could not recognize argument", err)

    def test_8_argument(self):
        """Verify 'calendar 8' does not fail"""
        code, out, err = self.t("calendar 8")
        self.assertNotIn("Could not recognize argument", err)

    def test_donkey_argument(self):
        """Verify 'calendar donkey' does fail"""
        code, out, err = self.t.runError("calendar donkey")
        self.assertIn("Could not recognize argument", err)

    def test_y_due_argument(self):
        """Verify 'calendar y due' does not fail"""
        code, out, err = self.t("calendar y due")
        self.assertNotIn("Could not recognize argument", err)

    def test_y_8_argument(self):
        """Verify 'calendar y 8' does not fail"""
        code, out, err = self.t("calendar y 8")
        self.assertNotIn("Could not recognize argument", err)

    def test_y_2015_argument(self):
        """Verify 'calendar y 2015' does not fail"""
        code, out, err = self.t("calendar y 2015")
        self.assertNotIn("Could not recognize argument", err)

    def test_y_donkey_argument(self):
        """Verify 'calendar y donkey' does fail"""
        code, out, err = self.t.runError("calendar y donkey")
        self.assertIn("Could not recognize argument", err)

    def test_8_due_argument(self):
        """Verify 'calendar 8 due' does not fail"""
        code, out, err = self.t("calendar 8 due")
        self.assertNotIn("Could not recognize argument", err)

    def test_8_2015_argument(self):
        """Verify 'calendar 8 2015' does not fail"""
        code, out, err = self.t("calendar 8 2015")
        self.assertNotIn("Could not recognize argument", err)

    def test_8_donkey_argument(self):
        """Verify 'calendar 8 donkey' does fail"""
        code, out, err = self.t.runError("calendar 8 donkey")
        self.assertIn("Could not recognize argument", err)

    def test_due_2015_argument(self):
        """Verify 'calendar due 2015' does not fail"""
        code, out, err = self.t("calendar due 2015")
        self.assertNotIn("Could not recognize argument", err)

    def test_due_donkey_argument(self):
        """Verify 'calendar due donkey' does fail"""
        code, out, err = self.t.runError("calendar due donkey")
        self.assertIn("Could not recognize argument", err)

    def test_january_argument(self):
        """Verify 'calendar january' does not fail"""
        code, out, err = self.t("calendar january")

    def test_jan_argument(self):
        """Verify 'calendar jan' does not fail"""
        code, out, err = self.t("calendar jan")

    def test_2015_argument(self):
        """Verify 'calendar 2015 donkey' does fail"""
        code, out, err = self.t.runError("calendar 2015 donkey")
        self.assertIn("Could not recognize argument", err)

    def test_y_8_due_argument(self):
        """Verify 'calendar y 8 due' does not fail"""
        code, out, err = self.t("calendar y 8 due")
        self.assertNotIn("Could not recognize argument", err)

    def test_y_8_2015_argument(self):
        """Verify 'calendary 8 2015' does not fail"""
        code, out, err = self.t("calendar y 8 2015")
        self.assertNotIn("Could not recognize argument", err)

    def test_y_8_donkey_argument(self):
        """Verify 'calendar y 8 donkey' does fail"""
        code, out, err = self.t.runError("calendar y 8 donkey")
        self.assertIn("Could not recognize argument", err)

    def test_y_due_2015_argument(self):
        """Verify 'calendar y due 2015' does not fail"""
        code, out, err = self.t("calendar y due 2015")
        self.assertNotIn("Could not recognize argument", err)

    def test_y_due_donkey_argument(self):
        """Verify 'calendar y due donkey' does fail"""
        code, out, err = self.t.runError("calendar y due donkey")
        self.assertIn("Could not recognize argument", err)

    def test_y_2015_donkey_argument(self):
        """Verify 'calendar y 2015 donkey' does fail"""
        code, out, err = self.t.runError("calendar y 2015 donkey")
        self.assertIn("Could not recognize argument", err)

    def test_8_due_2015_argument(self):
        """Verify 'calendar 8 due 2015' does not fail"""
        code, out, err = self.t("calendar 8 due 2015")
        self.assertNotIn("Could not recognize argument", err)

    def test_8_due_donkey_argument(self):
        """Verify 'calendar 8 due donkey' does fail"""
        code, out, err = self.t.runError("calendar 8 due donkey")
        self.assertIn("Could not recognize argument", err)

    def test_8_2015_donkey_argument(self):
        """Verify 'calendar 8 2015 donkey' does fail"""
        code, out, err = self.t.runError("calendar 8 2015 donkey")
        self.assertIn("Could not recognize argument", err)

    def test_due_2015_8_argument(self):
        """Verify 'calendar due 2015 8' does not fail"""
        code, out, err = self.t("calendar due 2015 8")
        self.assertNotIn("Could not recognize argument", err)

    def test_due_2015_donkey_argument(self):
        """Verify 'calendar due 2015 donkey' does fail"""
        code, out, err = self.t.runError("calendar due 2015 donkey")
        self.assertIn("Could not recognize argument", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
