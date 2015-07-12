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
from basetest.utils import UUID_REGEXP


class TestAdd(TestCase):
    def setUp(self):
        self.t = Task()

    def test_add(self):
        "Testing add command"

        self.t(("add", "This is a test"))
        code, out, err = self.t(("info", "1"))

        self.assertRegexpMatches(out, "ID\s+1\n")
        self.assertRegexpMatches(out, "Description\s+This is a test\n")
        self.assertRegexpMatches(out, "Status\s+Pending\n")
        self.assertRegexpMatches(out, "UUID\s+" + UUID_REGEXP + "\n")

    def test_modify_slash(self):
        "Test the /// modifier"

        self.t(("add", "This is a test"))
        self.t(("1", "modify", "/test/TEST/"))
        self.t(("1", "modify", "/is //"))

        code, out, err = self.t(("info", "1"))

        self.assertRegexpMatches(out, "ID\s+1\n")
        self.assertRegexpMatches(out, "Status\s+Pending\n")
        self.assertRegexpMatches(out, "Description\s+This a TEST\n")
        self.assertRegexpMatches(out, "UUID\s+" + UUID_REGEXP + "\n")

    def test_floating_point_preservation(self):
        """Verify that floating point numbers are unmolested"""
        # Bug 924: '1.0' --> '1.0000'
        self.t("add release 1.0")
        self.t("add 'release 2.0'")
        self.t("add \\\"release 3.0\\\"")

        code, out, err = self.t("_get 1.description")
        self.assertEqual(out, "release 1.0\n")

        code, out, err = self.t("_get 2.description")
        self.assertEqual(out, "release 2.0\n")

        code, out, err = self.t("_get 3.description")
        self.assertEqual(out, "release 3.0\n")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
