#!/usr/bin/env python3
###############################################################################
#
# Copyright 2025 Dustin J. Mitchell
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
import re
import time
import json
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import run_cmd_wait, CMAKE_BINARY_DIR


class TestUnusualTasks(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config(
            "report.custom-report.columns",
            "id,description,entry,start,end,due,scheduled,modified,until",
        )
        self.t.config("verbose", "nothing")

    def make_task(self, **props):
        make_tc_task = os.path.abspath(
            os.path.join(CMAKE_BINARY_DIR, "test", "make_tc_task")
        )
        cmd = [make_tc_task, self.t.datadir]
        for p, v in props.items():
            cmd.append(f"{p}={v}")
        _, out, _ = run_cmd_wait(cmd)
        return out.strip()

    def test_empty_task_info(self):
        uuid = self.make_task()
        _, out, _ = self.t(f"{uuid} info")
        self.assertNotIn("Entered", out)
        self.assertNotIn("Waiting", out)
        self.assertNotIn("Last modified", out)
        self.assertNotIn("Start", out)
        self.assertNotIn("End", out)
        self.assertNotIn("Due", out)
        self.assertNotIn("Until", out)
        self.assertRegex(out, r"Status\s+Pending")

    def test_modify_empty_task(self):
        uuid = self.make_task()
        self.t(f"{uuid} modify a description +taggy due:tomorrow")
        _, out, _ = self.t(f"{uuid} info")
        self.assertRegex(out, r"Description\s+a description")
        self.assertRegex(out, r"Tags\s+taggy")

    def test_empty_task_recurring(self):
        uuid = self.make_task(status="recurring")
        _, out, _ = self.t(f"{uuid} info")
        self.assertRegex(out, r"Status\s+Recurring")
        _, out, _ = self.t(f"{uuid} custom-report")

    def test_recurring_invalid_rtype(self):
        uuid = self.make_task(
            status="recurring", due=str(int(time.time())), rtype="occasional"
        )
        _, out, _ = self.t(f"{uuid} info")
        self.assertRegex(out, r"Status\s+Recurring")
        self.assertRegex(out, r"Recurrence type\s+occasional")
        _, out, _ = self.t(f"{uuid} custom-report")

    def test_recurring_invalid_recur(self):
        uuid = self.make_task(
            status="recurring",
            due=str(int(time.time())),
            rtype="periodic",
            recur="xxxxx",
        )
        _, out, _ = self.t(f"{uuid} info")
        self.assertRegex(out, r"Status\s+Recurring")
        self.assertRegex(out, r"Recurrence type\s+periodic")
        _, out, _ = self.t(f"{uuid} custom-report")

    def test_recurring_bad_quarters_rtype(self):
        uuid = self.make_task(
            status="recurring", due=str(int(time.time())), rtype="periodic", recur="9aq"
        )
        _, out, _ = self.t(f"{uuid} custom-report")

    def test_invalid_entry_info(self):
        uuid = self.make_task(entry="abcdef")
        _, out, _ = self.t(f"{uuid} info")
        self.assertNotIn("Entered", out)

    def test_invalid_modified_info(self):
        uuid = self.make_task(modified="abcdef")
        _, out, _ = self.t(f"{uuid} info")
        self.assertNotIn(r"Last modified", out)

    def test_invalid_start_info(self):
        uuid = self.make_task(start="abcdef")
        _, out, _ = self.t(f"{uuid} info")

    def test_invalid_dates_report(self):
        uuid = self.make_task(
            wait="wait",
            scheduled="scheduled",
            start="start",
            due="due",
            end="end",
            until="until",
            modified="modified",
        )
        _, out, _ = self.t(f"{uuid} custom-report")

    def test_invalid_dates_stop(self):
        uuid = self.make_task(
            wait="wait",
            scheduled="scheduled",
            start="start",
            due="due",
            end="end",
            until="until",
            modified="modified",
        )
        _, out, _ = self.t(f"{uuid} stop")

    def test_invalid_dates_modify(self):
        uuid = self.make_task(
            wait="wait",
            scheduled="scheduled",
            start="start",
            due="due",
            end="end",
            until="until",
            modified="modified",
        )
        _, out, _ = self.t(f"{uuid} mod a description +tag")

    def test_invalid_dates_info(self):
        uuid = self.make_task(
            wait="wait",
            scheduled="scheduled",
            start="start",
            due="due",
            end="end",
            until="until",
            modified="modified",
        )
        _, out, _ = self.t(f"{uuid} info")
        self.assertNotRegex("^Entered\s+", out)
        self.assertNotRegex("^Start\s+", out)
        self.assertIn(r"Wait set to 'wait'", out)
        self.assertIn(r"Scheduled set to 'scheduled'", out)
        self.assertIn(r"Start set to 'start'", out)
        self.assertIn(r"Due set to 'due'", out)
        self.assertIn(r"End set to 'end'", out)
        self.assertIn(r"Until set to 'until'", out)
        # (note that 'modified' is not shown in the journal)

    def test_invalid_dates_export(self):
        uuid = self.make_task(
            wait="wait",
            scheduled="scheduled",
            start="start",
            due="due",
            end="end",
            until="until",
            modified="modified",
        )
        _, out, _ = self.t(f"{uuid} export")
        json.loads(out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
