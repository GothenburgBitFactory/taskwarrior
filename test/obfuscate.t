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


class TestObfuscation(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t("add SECRET project:SECRET +SECRET")
        cls.t("1 annotate SECRET")

    def setUp(self):
        """Executed before each test in the class"""

    def test_info_obfuscation(self):
        """Verify that obfuscation hides all text in the 'info' command"""
        code, out, err = self.t("1 info")
        self.assertIn("SECRET", out)

        code, out, err = self.t("rc.obfuscate:1 1 info")
        self.assertIn("xxxxxx", out)
        self.assertNotIn("SECRET", out)

        code, out, err = self.t("rc.obfuscate:1 rc._forcecolor:1 1 info")
        self.assertIn("xxxxxx", out)
        self.assertNotIn("SECRET", out)

    def test_list_obfuscation(self):
        """Verify that obfuscation hides all text in a report"""
        code, out, err = self.t("list")
        self.assertIn("SECRET", out)

        code, out, err = self.t("rc.obfuscate:1 list")
        self.assertIn("xxxxxx", out)
        self.assertNotIn("SECRET", out)

        code, out, err = self.t("rc.obfuscate:1 rc._forcecolor:1 list")
        self.assertIn("xxxxxx", out)
        self.assertNotIn("SECRET", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
