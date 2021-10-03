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


class TestSummaryPercentage(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_summary(self):
        """Verify percentages on the summary report"""
        self.t("add project:A one")
        self.t("add project:A two")
        self.t("add project:A three")
        self.t("1 done")
        self.t("2 delete", input="y\n")
        code, out, err = self.t("summary")
        self.assertIn(" 50%", out)

        self.t("list")
        code, out, err = self.t("summary")
        self.assertIn(" 50%", out)

        code, out, err = self.t("summary rc._forcecolor:on")
        self.assertIn(" 50%", out)

    def test_summary_no_tasks(self):
        """Verify no tasks yields no report"""
        code, out, err = self.t.runError("summary")
        self.assertIn("No projects.", out)


class TestBug1904(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def add_tasks(self):
        self.t("add pro:a-b test1")
        self.t("add pro:a.b test2")

    def validate_order(self, out):
        order = ("a-b",
                 "a",
                 "  b")

        lines = out.splitlines(True)
        # position where project names start on the lines list
        position = 3

        for i, proj in enumerate(order):
            pos = position + i

            self.assertTrue(
                lines[pos].startswith(proj),
                msg=("Project '{0}' is not in line #{1} or has an unexpected "
                     "indentation.{2}".format(proj, pos, out))
            )

    def test_project_eval(self):
        """1904: verify correct order under summary command"""
        self.add_tasks()
        code, out, err = self.t("summary")
        self.validate_order(out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
