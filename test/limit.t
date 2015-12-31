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


class TestLimit(TestCase):

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_limit(self):
        """Verify limit:N works"""
        self.t.config("verbose", "affected")
        self.t("add one")
        self.t("add two")
        self.t("add three")
        self.t("add four")
        self.t("add five")
        self.t("add six")
        self.t("add seven")
        self.t("add eight")
        self.t("add nine")
        self.t("add ten")
        self.t("add eleven")
        self.t("add twelve")
        self.t("add thirteen")
        self.t("add fourteen")
        self.t("add fifteen")
        self.t("add sixteen")
        self.t("add severnteen")
        self.t("add eighteen")
        self.t("add nineteen")
        self.t("add twenty")
        self.t("add twenty one")
        self.t("add twenty two")
        self.t("add twenty three")
        self.t("add twenty four")
        self.t("add twenty five")
        self.t("add twenty six")
        self.t("add twenty seven")
        self.t("add twenty eight")
        self.t("add twenty nine")
        self.t("add thirty")

        code, out, err = self.t("ls")
        self.assertIn("30 tasks", out)

        code, out, err = self.t("ls limit:0")
        self.assertIn("30 tasks", out)

        code, out, err = self.t("ls limit:3")
        self.assertIn("30 tasks, 3 shown", out)

        # Default height is 24 lines:
        #   - header
        #   - blank
        #   - labels
        #   - underline
        #   - (data)
        #   - blank
        #   - affected
        #   - reserved.lines
        #  ------------
        #   = 17 lines

        code, out, err = self.t("ls limit:page")
        self.assertIn("30 tasks, truncated to 22 lines", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
