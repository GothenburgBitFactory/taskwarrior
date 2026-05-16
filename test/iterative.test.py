#!/usr/bin/env python3

import sys
import os
import json
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestIterativeStatusRecognized(TestCase):

    def setUp(self):
        self.t = Task()

    def test_add_with_iterative_status_succeeds(self):
        """task add status:iterative ... is accepted (textToStatus knows 'i')."""
        code, out, err = self.t("add status:iterative test")
        self.assertEqual(code, 0, msg=f"stderr: {err}")

    def test_iterative_status_roundtrips_via_export(self):
        """A task created with status:iterative exports with status:iterative."""
        self.t("add status:iterative test")
        code, out, err = self.t("export")
        tasks = json.loads(out)
        self.assertEqual(len(tasks), 1)
        self.assertEqual(tasks[0]["status"], "iterative")

    def test_stats_reports_iterative_row(self):
        """`task stats` includes an Iterative row and counts iterative tasks."""
        self.t("add status:iterative one")
        self.t("add pending two")
        code, out, err = self.t("stats")
        self.assertRegex(out, r"Iterative\s+1\n")
        self.assertRegex(out, r"Pending\s+1\n")
        self.assertRegex(out, r"Total\s+2\n")


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())
