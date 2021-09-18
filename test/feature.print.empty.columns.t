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


class TestPrintEmptyColumns(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()


    def test_empty_columns_feature(self):
        """Verify rc.print.empty.columns:yes shows more nothing than rc.print.empty.columns:no"""
        self.t("add one")
        self.t("add two project:work")

        # Should show empty 'Project' column.
        code, out, err = self.t("rc.print.empty.columns:yes /one/ list")
        self.assertIn("Project", out)

        # Should show populated 'Project' column.
        code, out, err = self.t("rc.print.empty.columns:yes /two/ list")
        self.assertIn("Project", out)

        # Should not show empty 'Project' column.
        code, out, err = self.t("rc.print.empty.columns:no /one/ list")
        self.assertNotIn("Project", out)

        # Should show populated 'Project' column.
        code, out, err = self.t("rc.print.empty.columns:no /two/ list")
        self.assertIn("Project", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
