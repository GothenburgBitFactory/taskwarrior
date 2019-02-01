#!/usr/bin/env python
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2019, Paul Beckingham, Federico Hernandez.
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


class TestUDAOrphans(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_orphan_handling(self):
        """Verify that orphans are preserved during various operations"""

        self.t("rc.uda.extra.type:string rc.uda.extra.label:Extra add one extra:foo")
        code, out, err = self.t("rc.uda.extra.type:string rc.uda.extra.label:Extra _get 1.extra")
        self.assertEqual("foo\n", out)

        # DOM access for orphans is not supported.
        self.t.runError("_get 1.extra")

        # 'info' should show orphans.
        code, out, err = self.t("1 info")
        self.assertRegex(out, "\[extra\s+foo\]")

        # 'modify' should not change the orphan
        self.t("1 modify /one/two/")
        code, out, err = self.t("1 info")
        self.assertRegex(out, "\[extra\s+foo\]")

        # 'export' should include orphans.
        code, out, err = self.t("1 export")
        self.assertIn('"extra":"foo"', out)

    def test_orphan_import(self):
        """Verify importing an orphan succeeds and is visible"""
        json = '{"description":"one","extra":"foo","status":"pending"}'
        code, out, err = self.t("import -", input=json)
        self.assertIn("Imported 1 tasks.", err)

        code, out, err = self.t("export")
        self.assertIn('"extra":"foo"', out)

    def test_orphan_creation_forbidden(self):
        """It should not be possible to create and orphan from the command line"""
        self.t("add one extra:foo")
        code, out, err = self.t("_get 1.description")
        self.assertIn("one extra:foo", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
