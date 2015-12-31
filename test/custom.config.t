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


class TestCustomConfig(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("alias.xyzzyx", "status:waiting")
        self.t.config("imnotrecognized", "kai")

        self.DIFFER_MSG = ("Some of your .taskrc variables differ from the "
                           "default values.")
        self.NOT_RECOG_MSG = ("Your .taskrc file contains these unrecognized "
                              "variables:")

    def test_show_alias(self):
        """task show <filter> - warns when non-default values are matched

        Reported in bug 1065
        """
        code, out, err = self.t("show alias")

        self.assertIn(self.DIFFER_MSG, out)
        self.assertNotIn(self.NOT_RECOG_MSG, out)

    def test_show(self):
        """task show - warns when non-default values are matched

        Reported in bug 1065
        """
        code, out, err = self.t("show")

        self.assertIn(self.DIFFER_MSG, out)
        self.assertIn(self.NOT_RECOG_MSG, out)

    def test_show_report_overdue(self):
        """task show <filter> - no warn when no non-default values are matched

        Reported in bug 1065
        """
        code, out, err = self.t("show report.overdue")

        self.assertNotIn(self.DIFFER_MSG, out)
        self.assertNotIn(self.NOT_RECOG_MSG, out)

    def test_show_notrecog(self):
        """task show <filter> - warns when unrecognized values are matched

        Reported in bug 1065
        """
        code, out, err = self.t("show notrecog")

        self.assertNotIn(self.DIFFER_MSG, out)
        self.assertIn(self.NOT_RECOG_MSG, out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
