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


class TestUdaReports(TestCase):
    @classmethod
    def setUp(cls):
        cls.t = Task()
        cls.t.config("uda.extra.label", "Extra")
        cls.t.config("uda.extra.type", "string")
        cls.t.config("report.good.columns", "id,extra")
        cls.t.config("report.good.description", "Test report")
        cls.t.config("report.good.filter", "")
        cls.t.config("report.good.labels", "ID,Extra")
        cls.t.config("report.good.sort", "id")
        cls.t.config("report.bad.columns", "id,extra2")
        cls.t.config("report.bad.description", "Test report2")
        cls.t.config("report.bad.filter", "")
        cls.t.config("report.bad.labels", "ID,Extra2")
        cls.t.config("report.bad.sort", "id")

        cls.t("add one extra:foo")

    def test_uda_show_report(self):
        """UDA shown in report"""

        code, out, err = self.t("good")
        self.assertIn("foo", out)

    def test_uda_no_show_report(self):
        """UDA not shown in report"""

        code, out, err = self.t.runError("bad")
        self.assertNotIn("foo", out)
        self.assertIn("Unrecognized column name 'extra2'.", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
