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


class TestBefore(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t('add foo entry:2008-12-22 start:2008-12-22')
        cls.t('add bar entry:2009-04-17 start:2009-04-17')

    def test_correctly_recorded_start(self):
        """Verify start dates properly recorded"""
        code, out, err = self.t("_get 1.start")
        self.assertEqual(out, "2008-12-22T00:00:00\n")

        code, out, err = self.t("_get 2.start")
        self.assertEqual(out, "2009-04-17T00:00:00\n")

    def test_before_none(self):
        """Verify start.before:2008-12-01 yields nothing"""
        code, out, err = self.t("start.before:2008-12-01 _ids")
        self.assertNotIn("1", out)
        self.assertNotIn("2", out)

    def test_after_none(self):
        """Verify start.after:2009-05-01 yields nothing"""
        code, out, err = self.t("start.after:2009-05-01 _ids")
        self.assertNotIn("1", out)
        self.assertNotIn("2", out)

    def test_before_a(self):
        """Verify start.before:2009-01-01 yields '1'"""
        code, out, err = self.t("start.before:2009-01-01 _ids")
        self.assertIn("1", out)
        self.assertNotIn("2", out)

    def test_before_b(self):
        """Verify start.before:2009-05-01 yields '1' and '2'"""
        code, out, err = self.t("start.before:2009-05-01 _ids")
        self.assertIn("1", out)
        self.assertIn("2", out)

    def test_after_a(self):
        """Verify start.after:2008-12-01 yields '1' and '2'"""
        code, out, err = self.t("start.after:2008-12-01 _ids")
        self.assertIn("1", out)
        self.assertIn("2", out)

    def test_after_b(self):
        """Verify start.after:2009-01-01 yields '2'"""
        code, out, err = self.t("start.after:2009-01-01 _ids")
        self.assertNotIn("1", out)
        self.assertIn("2", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
