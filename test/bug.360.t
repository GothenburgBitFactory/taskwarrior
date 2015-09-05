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
import re
import unittest
# Ensure python finds the local simpletap and basetest modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class BaseTestBug360(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add foo due:tomorrow recur:daily")
        # TODO: Add explanation why this line is necessary
        self.t("ls")


class TestBug360RemovalError(BaseTestBug360):
    def test_modify_recursive_project(self):
        """Modifying a recursive task by adding project: also modifies parent
        """
        code, out, err = self.t("1 modify project:bar", input="y\n")

        expected = "Modified 2 tasks."
        self.assertIn(expected, out)
        expected = "You cannot remove the recurrence from a recurring task."
        self.assertNotIn(expected, err)

    def test_cannot_remove_recurrence(self):
        """Cannot remove recurrence from recurring task
        """
        # TODO Removing recur: from a recurring task should also remove imask
        # and parent.

        code, out, err = self.t.runError("2 modify recur:")
        # Expected non zero exit-code
        self.assertEqual(code, 2)

        expected = "You cannot remove the recurrence from a recurring task."
        self.assertIn(expected, err)

    def test_cannot_remove_due_date(self):
        """Cannot remove due date from recurring task
        """
        # TODO Removing due: from a recurring task should also remove recur,
        # imask and parent
        code, out, err = self.t.runError("2 modify due:")
        # Expected non zero exit-code
        self.assertEqual(code, 2)

        expected = "You cannot remove the due date from a recurring task."
        self.assertIn(expected, err)


class TestBug360AllowedChanges(BaseTestBug360):
    def setUp(self):
        """Executed before each test in the class"""
        # Also do setUp from BaseTestBug360
        super(TestBug360AllowedChanges, self).setUp()

        self.t("add nonrecurring due:today")

    def test_allow_modify_due_in_nonrecurring(self):
        """Allow modifying due date in non recurring task"""
        # Retrieve the id of the non recurring task
        code, out, err = self.t("ls")

        expected = "2 tasks"
        self.assertIn(expected, out)

        # NOTE: raw python string r"" avoids having to escape backslashes
        id = re.search(r"(\d+)\s.+\snonrecurring", out).group(1)

        code, out, err = self.t((id, "modify", "due:"))

        expected = "Modified 1 task."
        self.assertIn(expected, out)
        expected = "You cannot remove the due date from a recurring task."
        self.assertNotIn(expected, err)

        # Make sure no duplicate tasks were created
        code, out, err = self.t.diag()
        expected = "No duplicates found"
        self.assertIn(expected, out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
