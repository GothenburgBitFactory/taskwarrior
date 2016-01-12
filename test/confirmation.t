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


class TestConfirmation(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_confirmation_response(self):
        """Verify confirmation works, and accepts appropriate responses"""
        self.t.config("confirmation", "on")

        # Create some playthings.
        for id in range(1,11):
            self.t("add foo")

        # Test the various forms of "Yes".
        code, out, err = self.t("1 delete", input="Yes\n")
        self.assertIn("Deleted 1 task.", out)
        code, out, err = self.t("2 delete", input="ye\n")
        self.assertIn("Deleted 1 task.", out)
        code, out, err = self.t("3 delete", input="y\n")
        self.assertIn("Deleted 1 task.", out)
        code, out, err = self.t("4 delete", input="YES\n")
        self.assertIn("Deleted 1 task.", out)
        code, out, err = self.t("5 delete", input="YE\n")
        self.assertIn("Deleted 1 task.", out)
        code, out, err = self.t("6 delete", input="Y\n")
        self.assertIn("Deleted 1 task.", out)

        # Test the various forms of "no".
        code, out, err = self.t.runError("7 delete", input="no\n")
        self.assertIn("Deleted 0 tasks.", out)
        code, out, err = self.t.runError("7 delete", input="n\n")
        self.assertIn("Deleted 0 tasks.", out)
        code, out, err = self.t.runError("7 delete", input="NO\n")
        self.assertIn("Deleted 0 tasks.", out)
        code, out, err = self.t.runError("7 delete", input="N\n")
        self.assertIn("Deleted 0 tasks.", out)

        # Blank repsonseѕ followed by a proper response.
        code, out, err = self.t.runError("7 delete", input="\n\nn\n")
        self.assertIn("Deleted 0 tasks.", out)


class TestBug1438(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_recurring_tasks_shouldn_ask_for_confirmation(self):
        """1438: rc.confirmation=off still prompts while changing recurring tasks"""
        code, out, err = self.t("add Sometimes due:tomorrow recur:daily")
        code, out, err = self.t("list")
        self.assertIn("Sometimes", out)

        code, out, err = self.t("rc.confirmation=off rc.recurrence.confirmation=off 2 mod /Sometimes/Everytime/")
        self.assertIn("Modified 1 task", out)
        code, out, err = self.t("list")
        self.assertIn("Everytime", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
