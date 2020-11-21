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


class TestInfoCommand(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_missing_info(self):
        """Verify bad filter yields error"""
        code, out, err = self.t.runError("999 info")
        self.assertIn("No matches.", err)

    def test_info_display(self):
        """Verify info command shows everything in the task"""
        self.t.config("uda.u_one.type", "date")
        self.t.config("uda.u_one.label", "U_ONE")
        self.t.config("uda.u_two.type", "duration")
        self.t.config("uda.u_two.label", "U_TWO")

        self.t.config("urgency.user.project.P.coefficient", "1.0")
        self.t.config("urgency.user.keyword.foo.coefficient", "1.0")
        self.t.config("urgency.uda.u_one.coefficient", "1.0")

        self.t("add foo project:P +tag priority:H start:now due:eom wait:eom scheduled:eom recur:P1M until:eoy u_one:now u_two:1day")
        self.t("1 annotate bar", input="n\n")
        code, out, err = self.t("1 info")

        self.assertRegex(out, "ID\s+1")
        self.assertRegex(out, "Description\s+foo")
        self.assertRegex(out, "\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}\s+bar")
        self.assertRegex(out, "Status\s+Recurring")
        self.assertRegex(out, "Project\s+P")
        self.assertRegex(out, "Recurrence\s+P1M")
        self.assertRegex(out, "Entered\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, "Waiting until\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, "Scheduled\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, "Start\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, "Due\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, "Until\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, "Last modified\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")

        self.assertRegex(out, "Tags\s+tag")
        self.assertIn("ACTIVE", out)
        self.assertIn("ANNOTATED", out)
        self.assertIn("MONTH", out)
        self.assertIn("SCHEDULED", out)
        self.assertIn("TAGGED", out)
        self.assertIn("UNBLOCKED", out)
        self.assertIn("UNTIL", out)
        self.assertIn("YEAR", out)
        self.assertIn("UDA", out)

        self.assertRegex(out, "UUID\s+[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}")
        self.assertRegex(out, "Urgency\s+\d+(\.\d+)?")
        self.assertRegex(out, "Priority\s+H")

        self.assertRegex(out, "Annotation of 'bar' added.")
        self.assertIn("project", out)
        self.assertIn("active", out)
        self.assertIn("annotations", out)
        self.assertIn("tags", out)
        self.assertIn("due", out)
        self.assertIn("UDA priority.H", out)
        self.assertIn("U_ONE", out)
        self.assertIn("U_TWO", out)

class TestBug425(TestCase):
    def setUp(self):
        self.t = Task()

    def test_bug425(self):
        """425: Make sure parser sees 'in' and not an abbreviated 'info'"""
        self.t("add Foo")
        self.t("1 modify Bar in Bar")

        code, out, err = self.t("1 ls")
        self.assertRegex(out, "1\s+Bar in Bar")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
