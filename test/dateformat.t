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


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
