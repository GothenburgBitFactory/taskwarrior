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


class TestHistoryMonthly(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

        self.data = """[
{"uuid":"00000000-0000-0000-0000-000000000000","description":"PLW","status":"pending","entry":"20150102T000000Z","wait":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000001","description":"PL","status":"pending","entry":"20150102T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000002","description":"DLN","status":"deleted","entry":"20150102T000000Z","end":"20150202T000000Z","due":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000003","description":"DLN2","status":"deleted","entry":"20150102T000000Z","end":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000004","description":"DNN","status":"deleted","entry":"20150102T000000Z","end":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000005","description":"CLN","status":"completed","entry":"20150102T000000Z","end":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000006","description":"CLL","status":"completed","entry":"20150102T000000Z","end":"20150102T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000007","description":"CNN","status":"completed","entry":"20150202T000000Z","end":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000008","description":"CNN2","status":"completed","entry":"20150202T000000Z","end":"20150202T000000Z"}
]"""
        self.t("import -", input=self.data)

    def test_history_monthly(self):
        """Verify 'history.monthly' correctly categorizes data"""
        code, out, err = self.t("history.monthly")
        self.assertRegexpMatches(out, "7\s+1\s+0\s+6")
        self.assertRegexpMatches(out, "2\s+3\s+3\s+-4")
        self.assertRegexpMatches(out, "4\s+2\s+1\s+1")

        code, out, err = self.t("ghistory.monthly rc._forcecolor:on")
        self.assertRegexpMatches(out, "\s7.+\s1.+")
        self.assertRegexpMatches(out, "\s2.+\s3.+\s3.+")

        code, out, err = self.t("ghistory.monthly")
        self.assertRegexpMatches(out, "2015 January\s+\++X+\s")
        self.assertRegexpMatches(out, "\s+February\s+\++X+\-+")


class TestHistoryAnnual(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

        self.data = """[
{"uuid":"00000000-0000-0000-0000-000000000000","description":"PLW","status":"pending","entry":"20140102T000000Z","wait":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000001","description":"PL","status":"pending","entry":"20140102T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000002","description":"DLN","status":"deleted","entry":"20140102T000000Z","end":"20150202T000000Z","due":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000003","description":"DLN2","status":"deleted","entry":"20140102T000000Z","end":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000004","description":"DNN","status":"deleted","entry":"20140102T000000Z","end":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000005","description":"CLN","status":"completed","entry":"20140102T000000Z","end":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000006","description":"CLL","status":"completed","entry":"20140102T000000Z","end":"20140102T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000007","description":"CNN","status":"completed","entry":"20150202T000000Z","end":"20150202T000000Z"},
{"uuid":"00000000-0000-0000-0000-000000000008","description":"CNN2","status":"completed","entry":"20150202T000000Z","end":"20150202T000000Z"}
]"""
        self.t("import -", input=self.data)

    def test_history_annual(self):
        """Verify 'history.annual' correctly categorizes data"""
        code, out, err = self.t("history.annual")
        self.assertRegexpMatches(out, "7\s+1\s+0\s+6")
        self.assertRegexpMatches(out, "2\s+3\s+3\s+-4")
        self.assertRegexpMatches(out, "4\s+2\s+1\s+1")

        code, out, err = self.t("ghistory.annual rc._forcecolor:on")
        self.assertRegexpMatches(out, "\s7.+\s1.+")
        self.assertRegexpMatches(out, "\s2.+\s3.+\s3.+")

        code, out, err = self.t("ghistory.annual")
        self.assertRegexpMatches(out, "2014\s+\++X+\s")
        self.assertRegexpMatches(out, "2015\s+\++X+\-+")

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
