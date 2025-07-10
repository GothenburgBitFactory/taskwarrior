#!/usr/bin/env python3
###############################################################################
#
# Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
import time
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

        self.t(
            "add foo project:P +tag priority:H start:now due:eom wait:eom scheduled:eom recur:P1M until:eoy u_one:now u_two:1day"
        )
        self.t("1 annotate bar", input="n\n")
        code, out, err = self.t("1 info")

        self.assertRegex(out, r"ID\s+1")
        self.assertRegex(out, r"Description\s+foo")
        self.assertRegex(out, r"\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}\s+bar")
        self.assertRegex(out, r"Status\s+Recurring")
        self.assertRegex(out, r"Project\s+P")
        self.assertRegex(out, r"Recurrence\s+P1M")
        self.assertRegex(out, r"Entered\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, r"Waiting until\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, r"Scheduled\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, r"Start\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, r"Due\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, r"Until\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")
        self.assertRegex(out, r"Last modified\s+\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}")

        self.assertRegex(out, r"Tags\s+tag")
        self.assertIn("ACTIVE", out)
        self.assertIn("ANNOTATED", out)
        self.assertIn("MONTH", out)
        self.assertIn("SCHEDULED", out)
        self.assertIn("TAGGED", out)
        self.assertIn("UNBLOCKED", out)
        self.assertIn("UNTIL", out)
        self.assertIn("YEAR", out)
        self.assertIn("UDA", out)

        self.assertRegex(
            out,
            r"UUID\s+[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}",
        )
        self.assertRegex(out, r"Urgency\s+\d+(\.\d+)?")
        self.assertRegex(out, r"Priority\s+H")

        self.assertRegex(out, r"Annotation of 'bar' added\.")
        self.assertRegex(out, r"Tag 'tag' added\.")
        self.assertRegex(out, r"tatus set to 'recurring'\.")
        self.assertIn("project", out)
        self.assertIn("active", out)
        self.assertIn("annotations", out)
        self.assertIn("tags", out)
        self.assertIn("due", out)
        self.assertIn("UDA priority.H", out)
        self.assertIn("U_ONE", out)
        self.assertIn("U_TWO", out)

        # TW-#2060: Make sure UDA attributes are formatted
        self.assertRegex(out, r"U_ONE\s+\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}")
        self.assertRegex(out, r"U_TWO\s+P1D")

    def test_tags_without_tags_attribute(self):
        """Verify info command shows tags, even if the `tags` property is not present"""
        # Create a task directly with TC, avoiding TaskWarrior's creation of the deprecated
        # `tags` property.
        uuid = self.t.make_tc_task(
            status="pending",
            description="task with tags",
            due=str(int(time.time())),
            tag_foo="x",
            tag_bar="x",
        )
        code, out, err = self.t("1 info")
        # Tags can occur in any order
        self.assertRegex(out, r"Tags\s+(bar|foo)\s(foo|bar)")


class TestBug425(TestCase):
    def setUp(self):
        self.t = Task()

    def test_bug425(self):
        """425: Make sure parser sees 'in' and not an abbreviated 'info'"""
        self.t("add Foo")
        self.t("1 modify Bar in Bar")

        code, out, err = self.t("1 ls")
        self.assertRegex(out, r"1\s+Bar in Bar")


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
