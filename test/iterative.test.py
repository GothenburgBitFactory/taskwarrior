#!/usr/bin/env python3

import sys
import os
import json
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestIterativeTasks(TestCase):
    """Iterative task support: status promotion, RRULE generation, alternative
    period strings, and stats reporting."""

    def setUp(self):
        self.t = Task()

    def test_iter_attribute_promotes_to_iterative_status(self):
        """`task add iter:weekly ...` -> status:iterative"""
        code, out, err = self.t("add iter:weekly iter_type:chained 'haircut'")
        self.assertEqual(code, 0, msg=f"stderr: {err}")
        code, out, err = self.t("export")
        tasks = json.loads(out)
        self.assertEqual(tasks[0]["status"], "iterative")

    def test_add_iterative_populates_rrule_and_due(self):
        """TaskChampion parses `iter` into an RRULE and computes the first
        `due` date on transition into iterative status."""
        code, out, err = self.t("add iter:weekly iter_type:fixed 'rent'")
        self.assertEqual(code, 0, msg=f"stderr: {err}")
        code, out, err = self.t("export")
        tasks = json.loads(out)
        self.assertEqual(len(tasks), 1)
        task = tasks[0]
        self.assertEqual(task["status"], "iterative")
        self.assertEqual(task["iter"], "weekly")
        self.assertEqual(task["iter_type"], "fixed")
        self.assertIn("FREQ=WEEKLY", task["rrule"])
        self.assertIn("due", task)

    def test_iter_alternative_period_strings(self):
        """str2rrule accepts a range of TaskWarrior-style period strings."""
        for iter_val, expected_freq in [
            ("daily", "FREQ=DAILY"),
            ("monthly", "FREQ=MONTHLY"),
            ("2week", "FREQ=WEEKLY"),
            ("fortnight", "FREQ=WEEKLY"),
        ]:
            with self.subTest(iter_val=iter_val):
                t = Task()
                code, out, err = t(f"add iter:{iter_val} iter_type:chained 'thing'")
                self.assertEqual(code, 0, msg=f"iter:{iter_val} stderr: {err}")
                code, out, err = t("export")
                self.assertIn(expected_freq, json.loads(out)[0]["rrule"])

    def test_iterative_status_without_iter_is_rejected(self):
        """Setting iterative status without an `iter` value is a usage error."""
        code, out, err = self.t.runError("add status:iterative 'no iter'")
        self.assertIn("Iterative tasks require an 'iter' value", err)

    def test_done_logs_completion_and_advances_due(self):
        """Marking an iterative task done creates a completed logged copy with
        `parent` pointing at the original, and the original stays iterative
        with `due` advanced to the next occurrence."""
        self.t("add iter:weekly iter_type:fixed 'rent'")
        orig_uuid = json.loads(self.t("status:iterative export")[1])[0]["uuid"]
        orig_due = json.loads(self.t("status:iterative export")[1])[0]["due"]
        self.t("rc.confirmation:off 1 done")
        all_tasks = json.loads(self.t("status.any: export")[1])
        self.assertEqual(len(all_tasks), 2)
        by_status = {t["status"]: t for t in all_tasks}
        self.assertIn("iterative", by_status)
        self.assertIn("completed", by_status)
        original = by_status["iterative"]
        logged = by_status["completed"]
        self.assertEqual(original["uuid"], orig_uuid)
        self.assertNotIn("end", original)
        self.assertGreater(original["due"], orig_due)  # advanced
        self.assertEqual(logged["parent"], orig_uuid)
        self.assertIn("end", logged)

    def test_stats_reports_iterative_row(self):
        """`task stats` reports an Iterative row alongside the other statuses."""
        self.t("add one")
        self.t("add iter:weekly 'do a thing'")
        self.t("add three")
        code, out, err = self.t("stats")
        self.assertRegex(out, r"Iterative\s+1\n")
        self.assertRegex(out, r"Pending\s+2\n")
        self.assertRegex(out, r"Total\s+3\n")


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())
