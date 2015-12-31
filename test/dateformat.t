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


class TestDateformat(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,due")
        cls.t.config("report.xxx.labels",  "ID,DUE")
        cls.t.config("report.xxx.sort",    "id")

    def setUp(self):
        """Executed before each test in the class"""

    def test_alternate_dateformats(self):
        """Verify a variety of dateformats elements succeed"""
        self.t("add foo rc.dateformat:'y/M/D'       due:15/07/04")
        self.t("add foo rc.dateformat:'m/d/y_h:n:s' due:7/4/15_0:0:0")
        self.t("add foo rc.dateformat:'YMDHNS'      due:20150704000000")

        code, out, err = self.t("xxx rc.dateformat:YMDTHNS")
        self.assertEqual(out.count("20150704T000000"), 3)

class TestBug886(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_invalid_day(self):
        """886: Test invalid day synonym

           Bug 886: tw doesn't warn the user if, e.g., a weekday cannot be resolved properly
        """
        code, out, err =self.t("add one due:sun")
        self.assertIn("Created task 1.", out)

        code, out, err =self.t.runError("add two due:donkey")
        self.assertIn("'donkey' is not a valid date", err)


class TestBug986(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_dateformat_precedence(self):
        """986: Verify rc.dateformat.info takes precedence over rc.dateformat"""
        self.t('add test')
        self.t('1 start')

        code, out, err = self.t('1 info rc.dateformat:XX rc.dateformat.info:__')
        self.assertIn('__', out)
        self.assertNotIn('XX', out)

        code, out, err = self.t('1 info rc.dateformat:__ rc.dateformat.info:')
        self.assertIn('__', out)


class TestBug1620(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config('dateformat', 'YMD-HN')

    def test_dateformat_overrides_iso(self):
        """1620: Verify that a defined dateformat overrides the ISO interpretation"""
        code, out, err = self.t ('add pro:vorhaben due:20150601-1415 tatusmeeting vorbereiten')

        code, out, err = self.t ('_get 1.due')
        self.assertEqual(out, "2015-06-01T14:15:00\n")

        code, out, err = self.t ('long')
        self.assertIn("20150601-1415", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
