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


class TestPeriod(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_recurrence_periods(self):
       """Verify recurrence period special-case support

       Date getNextRecurrence (Date& current, std::string& period)

       starting at line 509 special cases several possibilities for period, '\d\s?m'
       'monthly', 'quarterly', 'semiannual', 'bimonthly', 'biannual', 'biyearly'.
       Everything else falls through with period being passed to convertDuration.
       convertDuration doesn't know about 'daily' though so it seems to be returning 0.

       Confirmed:
         getNextRecurrence  convertDuration
         -----------------  ---------------
                            daily
                            day
                            weekly
                            sennight
                            biweekly
                            fortnight
         monthly            monthly
         quarterly          quarterly
         semiannual         semiannual
         bimonthly          bimonthly
         biannual           biannual
         biyearly           biyearly
                            annual
                            yearly
         *m                 *m
         *q                 *q
                            *d
                            *w
                            *y
       """

       self.t("add daily due:tomorrow recur:daily")
       self.t("add 1day due:tomorrow recur:1day")
       self.t("add weekly due:tomorrow recur:weekly")
       self.t("add 1sennight due:tomorrow recur:1sennight")
       self.t("add biweekly due:tomorrow recur:biweekly")
       self.t("add fortnight due:tomorrow recur:fortnight")
       self.t("add monthly due:tomorrow recur:monthly")
       self.t("add quarterly due:tomorrow recur:quarterly")
       self.t("add semiannual due:tomorrow recur:semiannual")
       self.t("add bimonthly due:tomorrow recur:bimonthly")
       self.t("add biannual due:tomorrow recur:biannual")
       self.t("add biyearly due:tomorrow recur:biyearly")
       self.t("add annual due:tomorrow recur:annual")
       self.t("add yearly due:tomorrow recur:yearly")
       self.t("add 2d due:tomorrow recur:2d")
       self.t("add 2w due:tomorrow recur:2w")
       self.t("add 2mo due:tomorrow recur:2mo")
       self.t("add 2q due:tomorrow recur:2q")
       self.t("add 2y due:tomorrow recur:2y")

       # Verify that the recurring task instances were created.  One of each.
       code, out, err = self.t("list")
       self.assertIn(" daily ",      out);
       self.assertIn(" 1day ",       out);
       self.assertIn(" weekly ",     out);
       self.assertIn(" 1sennight ",  out);
       self.assertIn(" biweekly ",   out);
       self.assertIn(" fortnight ",  out);
       self.assertIn(" monthly ",    out);
       self.assertIn(" quarterly ",  out);
       self.assertIn(" semiannual ", out);
       self.assertIn(" bimonthly ",  out);
       self.assertIn(" biannual ",   out);
       self.assertIn(" biyearly ",   out);
       self.assertIn(" annual ",     out);
       self.assertIn(" yearly ",     out);
       self.assertIn(" 2d ",         out);
       self.assertIn(" 2w ",         out);
       self.assertIn(" 2mo ",        out);
       self.assertIn(" 2q ",         out);
       self.assertIn(" 2y ",         out);

       # Duplicate check
       code, out, err = self.t("diag")
       self.assertIn("No duplicates found", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
