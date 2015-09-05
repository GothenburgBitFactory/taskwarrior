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


class TestBugBulk(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("bulk", "3")

        # Add some tasks with project, priority and due date, some with only
        # due date.  Bulk add a project and priority to the tasks that are
        # without.
        cls.t("add t1 pro:p1 pri:H due:monday")
        cls.t("add t2 pro:p1 pri:M due:tuesday")
        cls.t("add t3 pro:p1 pri:L due:wednesday")
        cls.t("add t4              due:thursday")
        cls.t("add t5              due:friday")
        cls.t("add t6              due:saturday")

    def setUp(self):
        """Executed before each test in the class"""

    def test_bulk_quit(self):
        """Verify 'quit' averts all bulk changes"""
        code, out, err = self.t("4 5 6 modify pro:p1 pri:M", input="quit\n")
        self.assertIn("Modified 0 tasks", out)

    def test_bulk_all(self):
        """Verify 'all' accepts all bulk changes"""
        code, out, err = self.t("4 5 6 modify pro:p1 pri:M", input="All\n")
        self.assertIn("Modifying task 4 't4'.", out)
        self.assertIn("Modifying task 5 't5'.", out)
        self.assertIn("Modifying task 6 't6'.", out)

        code, out, err = self.t("_get 4.project 5.project 6.project")
        self.assertEqual("p1 p1 p1\n", out)

        code, out, err = self.t("_get 4.priority 5.priority 6.priority")
        self.assertEqual("M M M\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
