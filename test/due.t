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
from datetime import datetime, timedelta
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


def timestamp_without_leading_zeros(time):
    t = time.strftime("%m/%d/%Y")
    return "/".join([x.lstrip("0") for x in t.split("/")])


class TestDue(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("due", "4")
        self.t.config("color", "on")
        self.t.config("color.due", "red")
        self.t.config("color.alternate", "")
        self.t.config("_forcecolor", "on")
        self.t.config("dateformat", "m/d/Y")

        just = datetime.now() + timedelta(days=3)
        almost = datetime.now() + timedelta(days=5)

        self.just = timestamp_without_leading_zeros(just)
        self.almost = timestamp_without_leading_zeros(almost)

        self.t("add one due:{0}".format(self.just))
        self.t("add two due:{0}".format(self.almost))

    def test_due(self):
        """due tasks displayed correctly"""
        code, out, err = self.t("list")
        self.assertRegexpMatches(out, "\033\[31m.+{0}.+\033\[0m".format(self.just))
        self.assertRegexpMatches(out, "\s+{0}\s+".format(self.almost))


class TestBug418(TestCase):
    # NOTE: Originally Bug #418: due.before:eow not working
    #   - with dateformat 'MD'
    def setUp(self):
        self.t = Task()

        self.t.config("due", "4")
        self.t.config("dateformat", "m/d/Y")
        self.t.config("report.foo.description", "Sample")
        self.t.config("report.foo.columns", "id,due,description")
        self.t.config("report.foo.labels", "ID,Due,Description")
        self.t.config("report.foo.sort", "due+")
        self.t.config("report.foo.filter", "status:pending")
        self.t.config("report.foo.dateformat", "MD")

    def test_bug_418(self):
        """due.before:eow bad with dateformat 'MD'"""
        self.t("add one   due:6/28/2010")
        self.t("add two   due:6/29/2010")
        self.t("add three due:6/30/2010")
        self.t("add four  due:7/1/2010")
        self.t("add five  due:7/2/2010")
        self.t("add six   due:7/3/2010")
        self.t("add seven due:7/4/2010")
        self.t("add eight due:7/5/2010")
        self.t("add nine  due:7/6/2010")

        code, out, err = self.t("foo")
        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)
        self.assertIn("eight", out)
        self.assertIn("nine", out)

        code, out, err = self.t("foo due.before:7/2/2010")
        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("three", out)
        self.assertIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)
        self.assertNotIn("eight", out)
        self.assertNotIn("nine", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
