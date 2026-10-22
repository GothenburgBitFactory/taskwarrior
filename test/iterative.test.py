#!/usr/bin/env python3

import sys
import os
import json
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import mkstemp_exec


class TestIterativeTasks(TestCase):
    """Iterative task support: status changes, series behavior, built-in
    properties, and reporting."""

    def setUp(self):
        self.t = Task()

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

    def test_iter_multi_word_value(self):
        """A quoted multi-word `iter` value is kept as one value."""
        code, out, err = self.t(
            "add iter:'every monday' iter_type:chained due:tomorrow 'thing'"
        )
        self.assertEqual(code, 0, msg=f"stderr: {err}")
        task = json.loads(self.t("export")[1])[0]
        self.assertEqual(task["status"], "iterative")
        self.assertEqual(task["iter"], "every monday")

    def test_iterative_status_without_iter_is_rejected(self):
        """Setting iterative status without an `iter` value is a usage error."""
        code, out, err = self.t.runError("add status:iterative 'no iter'")
        self.assertIn("Iterative tasks require an 'iter' value", err)

    def test_done_logs_completion_and_advances_due(self):
        """Marking an iterative task done completes it and spawns an iterative
        successor with the next `iter_count` and `due` advanced."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        orig_due = json.loads(self.t("export")[1])[0]["due"]
        self.t("rc.confirmation:off 1 done")
        all_tasks = json.loads(self.t("status.any: export")[1])
        self.assertEqual(len(all_tasks), 2)
        by_status = {t["status"]: t for t in all_tasks}
        self.assertEqual(by_status["completed"]["iter_count"], 1)
        successor = by_status["iterative"]
        self.assertEqual(successor["iter_count"], 2)
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
        tasks and recurring templates, and include a plain iterative task."""
        self.t("add wait:tomorrow 'plain waiting'")
        self.t("add iter:weekly iter_type:fixed wait:tomorrow due:2wk 'iter waiting'")
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'iter open'")
        self.t("add recur:weekly due:tomorrow 'template'")
        code, out, err = self.t("status:recurring +PENDING count")
        self.assertEqual(out.strip(), "0")
        code, out, err = self.t("+PENDING -INSTANCE export")
        self.assertEqual([t["description"] for t in json.loads(out)], ["iter open"])
        code, out, err = self.t("+READY -INSTANCE export")
        self.assertEqual([t["description"] for t in json.loads(out)], ["iter open"])

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

    def test_modify_iteration_data(self):
        """Removing iter_type or the last anchor date is rejected, moving the
        anchor is allowed, and clearing iter ends the series."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        code, out, err = self.t.runError("1 modify iter_type:")
        self.assertIn(
            "You cannot remove the iteration type from an iterative task.", err
        )
        code, out, err = self.t.runError("1 modify due:")
        self.assertIn(
            "You cannot remove the last of the due, scheduled and wait dates", err
        )
        self.t("1 modify scheduled:tomorrow due:")
        task = json.loads(self.t("export")[1])[0]
        self.assertEqual(task["status"], "iterative")
        self.assertNotIn("due", task)
        self.t("1 modify iter:")
        task = json.loads(self.t("export")[1])[0]
        self.assertEqual(task["status"], "pending")
        self.assertNotIn("iter", task)
        self.assertNotIn("iter_type", task)
        self.assertNotIn("iter_count", task)
        self.t("rc.confirmation:off 1 done")
        code, out, err = self.t("status.any: count")
        self.assertEqual(out.strip(), "1")

    def test_cleared_series_restarts_at_one(self):
        """Clearing `iter` on a later task and adding it back starts a new
        series at `iter_count` 1."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        self.t("rc.confirmation:off 1 done")
        self.t("+ITERATIVE modify iter:")
        self.t("+PENDING modify iter:weekly iter_type:fixed")
        task = json.loads(self.t("+ITERATIVE export")[1])[0]
        self.assertEqual(task["iter_count"], 1)

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

    def test_iter_properties_are_builtin(self):
        """`iter` and `iter_type` are built-in columns, not UDAs."""
        code, out, err = self.t("columns iter")
        self.assertIn("iter_type", out)
        self.assertRegex(out, r"\biter\b")

    def test_iter_type_values(self):
        """`iter_type` accepts the TaskChampion types and rejects others."""
        code, out, err = self.t.runError(
            "add iter:weekly iter_type:bogus due:tomorrow 'rent'"
        )
        self.assertIn(
            "The 'iter_type' attribute does not allow a value of 'bogus'.", err
        )
        self.t("add iter:weekly iter_type:fixed+ due:tomorrow 'rent'")
        task = json.loads(self.t("export")[1])[0]
        self.assertEqual(task["iter_type"], "fixed+")

    def test_edit_changes_iter_properties(self):
        """`task edit` shows and parses `iter` and `iter_type`."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        editor = mkstemp_exec(
            b"sed -i -e 's/^  Iter: .*/  Iter:              daily/' "
            b"-e 's/^  Iter type: .*/  Iter type:         chained/' $1\n"
        )
        self.t.env["VISUAL"] = editor
        self.t("1 edit")
        task = json.loads(self.t("export")[1])[0]
        self.assertEqual(task["iter"], "daily")
        self.assertEqual(task["iter_type"], "chained")

    def test_edit_rejects_iterative_inconsistencies(self):
        """`task edit` applies the same iterative checks as `modify`."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        for sed, message in [
            (
                b"s/^  Recur: .*/  Recur:             weekly/",
                "A task cannot be both recurring and iterative.",
            ),
            (
                b"s/^  Iter type: .*/  Iter type:/",
                "You cannot remove the iteration type from an iterative task.",
            ),
        ]:
            with self.subTest(message=message):
                self.t.env["VISUAL"] = mkstemp_exec(b"sed -i -e '" + sed + b"' $1\n")
                code, out, err = self.t("1 edit", input="n\n")
                self.assertIn(message, err)
                task = json.loads(self.t("export")[1])[0]
                self.assertEqual(task["status"], "iterative")
                self.assertNotIn("recur", task)
                self.assertEqual(task["iter_type"], "fixed")

    def test_rejects_recur_with_iter(self):
        """A task cannot be added with both `recur` and `iter`, `iter` cannot be
        added to a recurring template or instance, and `recur` cannot be added
        to an iterative task."""
        code, out, err = self.t.runError(
            "add recur:weekly iter:weekly iter_type:fixed due:tomorrow 'rent'"
        )
        self.assertIn("A task cannot be both recurring and iterative.", err)
        self.t("add recur:weekly due:tomorrow 'template'")
        self.t("list")
        code, out, err = self.t.runError(
            "rc.recurrence.confirmation:no status:recurring modify iter:weekly iter_type:fixed"
        )
        self.assertIn("A task cannot be both recurring and iterative.", err)
        code, out, err = self.t.runError(
            "rc.recurrence.confirmation:no +INSTANCE modify iter:weekly iter_type:fixed"
        )
        self.assertIn("A task cannot be both recurring and iterative.", err)
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        code, out, err = self.t.runError("+ITERATIVE modify recur:weekly")
        self.assertIn("A task cannot be both recurring and iterative.", err)

    def test_stats_reports_iterative_row(self):
        """`task stats` reports an Iterative row alongside the other statuses."""
        self.t("add one")
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'do a thing'")
        self.t("add three")
        code, out, err = self.t("stats")
        self.assertRegex(out, r"Iterative\s+1\n")
        self.assertRegex(out, r"Pending\s+2\n")
        self.assertRegex(out, r"Total\s+3\n")

    def test_info_shows_iter_properties(self):
        """`info` shows `iter`, `iter_type` and `iter_count`, and none of them
        are orphaned UDAs."""
        self.t("add iter:weekly iter_type:fixed due:tomorrow 'rent'")
        code, out, err = self.t("1 info")
        self.assertRegex(out, r"Iter\s+weekly")
        self.assertRegex(out, r"Iter type\s+fixed")
        self.assertRegex(out, r"Iter Count\s+1")
        code, out, err = self.t("+ORPHAN count")
        self.assertEqual(out.strip(), "0")


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())
