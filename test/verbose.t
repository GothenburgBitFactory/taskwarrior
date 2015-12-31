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
import re
import unittest
import operator

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestVerbosity(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("print.empty.columns", "yes")

        self.t("add Sample")

    # TODO Verbosity: 'edit'

    def test_verbosity_new_id(self):
        """Verbosity new-id"""
        code, out, err = self.t("rc.verbose:new-id add Sample1")
        self.assertRegexpMatches(out, r"Created task \d")

        code, out, err = self.t("rc.verbose:nothing add Sample2")
        self.assertNotRegexpMatches(out, r"Created task \d")

    def test_verbosity_new_uuid(self):
        """Verbosity new-uuid"""
        code, out, err = self.t(("rc.verbose:new-uuid", "add", "Sample1"))
        self.assertRegexpMatches(out, r"Created task [0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}")

    def test_verbosity_label(self):
        """Verbosity label"""
        code, out, err = self.t("rc.verbose:label ls")
        self.assertRegexpMatches(
            out,
            "ID.+A.+D.+Project.+Tags.+R.+Wait.+S.+Due.+Until.+Description"
        )

    def test_verbosity_affected(self):
        """Verbosity affected"""
        code, out, err = self.t("rc.verbose:affected ls")

        expected = re.compile(r"^\d+ tasks?$", re.MULTILINE)
        self.assertRegexpMatches(out, expected)

    def test_verbosity_off(self):
        """Verbosity off"""
        code, out, err = self.t("rc.verbose:nothing ls")

        expected = re.compile(r"^\d+ tasks?$", re.MULTILINE)
        self.assertNotRegexpMatches(out, expected)
        self.assertNotRegexpMatches(out, "ID.+Project.+Pri.+Description")

    def test_verbosity_special(self):
        """Verbosity special"""
        code, out, err = self.t("rc.verbose:special 1 mod +next")

        self.assertIn("The 'next' special tag will boost the urgency of this "
                      "task so it appears on the 'next' report.", out)

    def test_verbosity_blank(self):
        """Verbosity blank"""

        def count_blank_lines(x):
            return len(filter(operator.not_, x.splitlines()))

        code, out, err = self.t("rc.verbose:nothing ls")
        self.assertEqual(count_blank_lines(out), 0)

        code, out, err = self.t("rc.verbose:blank ls")
        self.assertEqual(count_blank_lines(out), 2)

    def test_verbosity_header(self):
        """Verbosity header"""

        code, out, err = self.t("rc.verbose:nothing ls")
        self.assertNotIn("TASKRC override:", err)
        self.assertNotIn("TASKDATA override:", err)

        code, out, err = self.t("rc.verbose:header ls")
        self.assertIn("TASKRC override:", err)
        self.assertIn("TASKDATA override:", err)

    def test_verbosity_project(self):
        """Verbosity project"""

        code, out, err = self.t("rc.verbose:nothing add proj:T one")
        self.assertNotIn("The project 'T' has changed.", err)

        code, out, err = self.t("rc.verbose:project add proj:T two")
        self.assertIn("The project 'T' has changed.", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
