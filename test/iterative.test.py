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
        code, out, err = self.t(
            "add iter:weekly iter_type:chained due:tomorrow 'haircut'"
        )
        self.assertEqual(code, 0, msg=f"stderr: {err}")
        code, out, err = self.t("export")
        tasks = json.loads(out)
        self.assertEqual(tasks[0]["status"], "iterative")

    def test_add_iterative_sets_iter_count(self):
        """Adding an iterative task keeps the given `due` as the first
        occurrence and starts the series at `iter_count` 1."""
        code, out, err = self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        self.assertEqual(code, 0, msg=f"stderr: {err}")
        code, out, err = self.t("export")
        tasks = json.loads(out)
        self.assertEqual(len(tasks), 1)
        task = tasks[0]
        self.assertEqual(task["status"], "iterative")
        self.assertEqual(task["iter"], "weekly")
        self.assertEqual(task["iter_type"], "fixed")
        self.assertEqual(task["iter_count"], 1)
        self.assertIn("due", task)

    def test_iter_alternative_period_strings(self):
        """A range of TaskWarrior-style period strings are accepted as `iter`."""
        for iter_val in ["daily", "monthly", "2week", "fortnight"]:
            with self.subTest(iter_val=iter_val):
                t = Task()
                code, out, err = t(
                    f"add iter:{iter_val} iter_type:chained due:tomorrow 'thing'"
                )
                self.assertEqual(code, 0, msg=f"iter:{iter_val} stderr: {err}")
                code, out, err = t("export")
                task = json.loads(out)[0]
                self.assertEqual(task["status"], "iterative")
                self.assertEqual(task["iter"], iter_val)

    def test_iterative_status_without_iter_is_rejected(self):
        """Setting iterative status without an `iter` value is a usage error."""
        code, out, err = self.t.runError("add status:iterative 'no iter'")
        self.assertIn("Iterative tasks require an 'iter' value", err)

    def test_done_logs_completion_and_advances_due(self):
        """Marking an iterative task done completes the original in place and
        spawns a new iterative successor with the next `iter_count` and its
        `due` advanced to the next occurrence."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        orig = json.loads(self.t("status:iterative export")[1])[0]
        orig_uuid = orig["uuid"]
        orig_due = orig["due"]
        self.t("rc.confirmation:off 1 done")
        all_tasks = json.loads(self.t("status.any: export")[1])
        self.assertEqual(len(all_tasks), 2)
        by_status = {t["status"]: t for t in all_tasks}
        self.assertIn("iterative", by_status)
        self.assertIn("completed", by_status)
        successor = by_status["iterative"]
        logged = by_status["completed"]
        # The original keeps its uuid and position, gains `end`, and is no
        # longer iterative.
        self.assertEqual(logged["uuid"], orig_uuid)
        self.assertIn("end", logged)
        self.assertEqual(logged["iter_count"], 1)
        self.assertNotIn("iter", logged)
        self.assertNotIn("iter_type", logged)
        # The successor is a new task carrying the schedule, due advanced.
        self.assertNotEqual(successor["uuid"], orig_uuid)
        self.assertEqual(successor["iter_count"], 2)
        self.assertEqual(successor["iter"], "weekly")
        self.assertEqual(successor["iter_type"], "fixed")
        self.assertNotIn("end", successor)
        self.assertGreater(successor["due"], orig_due)

    def test_duplicate_iterative_starts_new_series(self):
        """Duplicating a later occurrence starts a new series at `iter_count` 1
        and keeps the schedule."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        self.t("rc.confirmation:off 1 done")
        successor = json.loads(self.t("status:iterative export")[1])[0]
        self.assertEqual(successor["iter_count"], 2)
        self.t("rc.confirmation:off 1 duplicate")
        tasks = json.loads(self.t("status:iterative export")[1])
        self.assertEqual(len(tasks), 2)
        dup = next(t for t in tasks if t["uuid"] != successor["uuid"])
        self.assertEqual(dup["iter_count"], 1)
        self.assertEqual(dup["iter"], "weekly")
        self.assertEqual(dup["iter_type"], "fixed")
        self.assertEqual(dup["due"], successor["due"])

    def test_iterative_task_appears_in_list_and_next(self):
        """Default report filters surface iterative tasks alongside pending."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        self.t("add 'plain'")
        code, out, err = self.t("list")
        self.assertIn("rent", out)
        self.assertIn("plain", out)
        code, out, err = self.t("next")
        self.assertIn("rent", out)
        self.assertIn("plain", out)

    def test_iterative_task_with_future_wait_is_waiting(self):
        """An iterative task with a future wait is hidden like a waiting task."""
        self.t("add iter:weekly iter_type:fixed wait:tomorrow due:2wk 'rent'")
        code, out, err = self.t.runError("list")
        self.assertNotIn("rent", out)
        code, out, err = self.t("waiting")
        self.assertIn("rent", out)
        code, out, err = self.t("+WAITING count")
        self.assertEqual(out.strip(), "1")

    def test_pending_and_ready_exclude_waiting(self):
        """+PENDING and +READY exclude waiting pending and waiting iterative
        tasks, and include a plain iterative task."""
        self.t("add wait:tomorrow 'plain waiting'")
        self.t("add iter:weekly iter_type:fixed wait:tomorrow due:2wk 'iter waiting'")
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'iter open'")
        code, out, err = self.t("+PENDING export")
        self.assertEqual([t["description"] for t in json.loads(out)], ["iter open"])
        code, out, err = self.t("+READY export")
        self.assertEqual([t["description"] for t in json.loads(out)], ["iter open"])

    def test_pending_excludes_recurring_template(self):
        """+PENDING does not match a recurring template."""
        self.t("add recur:weekly due:tomorrow 'template'")
        code, out, err = self.t("status:recurring +PENDING count")
        self.assertEqual(out.strip(), "0")

    def test_waiting_task_promoted_to_iterative(self):
        """Adding `iter` to a waiting task makes it iterative."""
        self.t("add wait:tomorrow due:2wk 'rent'")
        self.t("1 modify iter:weekly iter_type:fixed")
        task = json.loads(self.t("export")[1])[0]
        self.assertEqual(task["status"], "iterative")
        self.assertEqual(task["iter_count"], 1)

    def test_status_column_renders_iterative(self):
        """The status column renders iterative tasks in both styles."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        self.t.config("report.st.columns", "id,status,status.short,description")
        self.t.config("report.st.filter", "status:iterative")
        code, out, err = self.t("st")
        self.assertRegex(out, r"1\s+Iterative\s+I\s+rent")

    def test_add_and_duplicate_report_new_id(self):
        """Adding or duplicating an iterative task reports the new ID."""
        code, out, err = self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        self.assertIn("Created task 1.", out)
        code, out, err = self.t("rc.confirmation:off 1 duplicate")
        self.assertIn("Created task 2.", out)

    def test_modify_cannot_remove_iteration_data(self):
        """Removing iter, iter_type or the last anchor date is rejected."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        code, out, err = self.t.runError("1 modify iter:")
        self.assertIn("You cannot remove the iteration from an iterative task.", err)
        code, out, err = self.t.runError("1 modify iter_type:")
        self.assertIn(
            "You cannot remove the iteration type from an iterative task.", err
        )
        code, out, err = self.t.runError("1 modify due:")
        self.assertIn(
            "You cannot remove the last of the due, scheduled and wait dates", err
        )

    def test_modify_can_move_anchor_date(self):
        """Removing one anchor date is allowed while another remains."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        self.t("1 modify scheduled:tomorrow due:")
        task = json.loads(self.t("export")[1])[0]
        self.assertEqual(task["status"], "iterative")
        self.assertNotIn("due", task)

    def test_calendar_due_starts_at_iterative_due_date(self):
        """`task calendar due` starts at an iterative task's due date."""
        self.t("add iter:weekly iter_type:fixed due:2030-06-15 'rent'")
        code, out, err = self.t("calendar due")
        self.assertIn("June 2030", out)

    def test_summary_counts_iterative_as_remaining(self):
        """`task summary` counts an iterative task as remaining."""
        self.t("add project:home iter:weekly iter_type:fixed due:tomorrow 'rent'")
        code, out, err = self.t("summary")
        self.assertRegex(out, r"home\s+1\s")

    def test_project_feedback_counts_iterative(self):
        """Project completion feedback counts iterative tasks as remaining."""
        code, out, err = self.t(
            "add project:home iter:weekly iter_type:fixed due:tomorrow 'rent'"
        )
        self.assertIn("(1 task remaining)", err)
        code, out, err = self.t("rc.confirmation:off 1 done")
        self.assertIn("is 50% complete (1 of 2 tasks remaining)", err)

    def test_modify_cannot_set_end_on_iterative(self):
        """Setting an end date on an iterative task is rejected."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        code, out, err = self.t.runError("1 modify end:now")
        self.assertIn("You cannot set an end date on a pending task.", err)

    def test_import_drops_end_from_iterative(self):
        """An imported iterative task loses its end date."""
        self.t(
            "import",
            input='{"uuid":"a0000000-0000-4000-8000-000000000001","status":"iterative",'
            '"description":"rent","iter":"weekly","iter_type":"fixed",'
            '"due":"20300615T000000Z","end":"20260101T000000Z"}',
        )
        task = json.loads(self.t("export")[1])[0]
        self.assertEqual(task["status"], "iterative")
        self.assertNotIn("end", task)

    def test_stats_reports_iterative_row(self):
        """`task stats` reports an Iterative row alongside the other statuses."""
        self.t("add one")
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'do a thing'")
        self.t("add three")
        code, out, err = self.t("stats")
        self.assertRegex(out, r"Iterative\s+1\n")
        self.assertRegex(out, r"Pending\s+2\n")
        self.assertRegex(out, r"Total\s+3\n")

    def test_iter_count_is_not_an_orphan(self):
        """`iter_count` is a built-in attribute, not an orphaned UDA."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        code, out, err = self.t("+ORPHAN count")
        self.assertEqual(out.strip(), "0")
        code, out, err = self.t("1 info")
        self.assertRegex(out, r"Iter Count\s+1")


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())
