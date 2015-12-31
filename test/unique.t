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


class TestUnique(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t("add one project:A")
        cls.t("add two project:A")
        cls.t("add three project:B")
        cls.t("add four project:C")
        cls.t("4 delete", input="y\n")
        cls.t("log five project:D")

    def setUp(self):
        """Executed before each test in the class"""

    def test_unique_projects(self):
        """Verify that unique projects are correctly counted"""

        code, out, err = self.t("_unique project")
        self.assertIn("A", out)
        self.assertIn("B", out)
        self.assertIn("C", out)
        self.assertIn("D", out)

        code, out, err = self.t("status:pending _unique project")
        self.assertIn("A", out)
        self.assertIn("B", out)
        self.assertNotIn("C", out)
        self.assertNotIn("D", out)

    def test_unique_status(self):
        """Verify that unique status values are correctly counted"""
        code, out, err = self.t("_unique status")
        self.assertIn("pending",   out)
        self.assertIn("completed", out)
        self.assertIn("deleted",   out)

    def test_unique_description(self):
        """Verify that unique description values are correctly counted"""
        code, out, err = self.t("_unique description")
        self.assertIn("one",   out)
        self.assertIn("two",   out)
        self.assertIn("three", out)
        self.assertIn("four",  out)
        self.assertIn("five",  out)

    def test_unique_id(self):
        """Verify that unique id values are correctly counted"""
        code, out, err = self.t("_unique id")
        self.assertIn("1", out)
        self.assertIn("2", out)
        self.assertIn("3", out)
        self.assertNotIn("4", out)
        self.assertNotIn("5", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
