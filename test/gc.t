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


class TestGC(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("report.gctest.description", "gctest")
        self.t.config("report.gctest.columns", "id,description,tags")
        self.t.config("report.gctest.sort", "id+")
        self.t("add one")
        self.t("add two")
        self.t("add three")

    def test_gc_off_id(self):
        """ID retained when GC off"""
        self.t.config("gc", "off")
        self.t("1 done")
        code, out, err = self.t("gctest")
        self.assertRegexpMatches(out, "1\s+one", "should still have ID")

    def test_gc_off_mod(self):
        """mod by ID after done with gc off"""
        self.t.config("gc", "off")
        self.t("1 done")
        self.t("gctest")
        self.t("2 mod +TWO")
        code, out, err = self.t("gctest")
        self.assertRegexpMatches(out, "2\s+two\s+TWO", "modified 'two'")

    def test_gc_on_id(self):
        """IDs reshuffle after report when GC on"""
        self.t.config("gc", "on")
        self.t("1 done")
        self.t("2 mod +TWO")
        code, out, err = self.t("gctest")
        self.assertRegexpMatches(out, "1\s+two\s+TWO")
        self.assertRegexpMatches(out, "2\s+three")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
