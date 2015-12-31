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


class TestDiagColor(TestCase):
    def setUp(self):
        self.t = Task()

    def test_diag_color_builtin(self):
        """Task diag detects terminal as color compatible with test-builtin"""
        code, out, err = self.t.diag()

        expected = "\x1b[1m"
        self.assertNotIn(expected, out)

    def test_diag_color(self):
        """Task diag detects terminal as color compatible"""
        code, out, err = self.t("diag")

        expected = "\x1b[1m"
        self.assertNotIn(expected, out)

    def test_diag_nocolor(self):
        """Task diag respects rc:color=off and disables color"""
        code, out, err = self.t("rc.color:off diag")

        expected = "\x1b[1m"
        self.assertNotIn(expected, out)

    def test_diag_force_color(self):
        """Task diag respects rc:_forcecolor=on and forces color"""
        code, out, err = self.t("rc._forcecolor:on diag")

        expected = "\x1b[1m"
        self.assertIn(expected, out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
