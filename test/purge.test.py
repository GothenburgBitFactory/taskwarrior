#!/usr/bin/env python3
###############################################################################
#
# Copyright 2006 - 2024, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
import time
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import mkstemp


class TestAutoPurge(TestCase):
    def setUp(self):
        self.t = Task()
        # Set up local sync within the TASKDATA directory, so that it will be
        # deleted properly.
        self.t.config("sync.local.server_dir", self.t.datadir)

    def exists(self, uuid):
        code, out, err = self.t(f"_get {uuid}.status")
        return out.strip() != ""

    def test_auto_purge(self):
        """Only tasks that are deleted and have a modification in the past are purged."""
        yesterday = int(time.time()) - 3600 * 24
        last_year = int(time.time()) - 265 * 3600 * 24
        old_pending = "a1111111-a111-a111-a111-a11111111111"
        old_completed = "a2222222-a222-a222-a222-a22222222222"
        new_deleted = "a3333333-a333-a333-a333-a33333333333"
        old_deleted = "a4444444-a444-a444-a444-a44444444444"
        task_data = f"""[
    {{"uuid":"{old_pending}","status":"pending","modified":"{last_year}","description":"x"}},
    {{"uuid":"{old_completed}","status":"completed","modified":"{last_year}","description":"x"}},
    {{"uuid":"{new_deleted}","status":"deleted","modified":"{yesterday}","description":"x"}},
    {{"uuid":"{old_deleted}","status":"deleted","modified":"{last_year}","description":"x"}}
]
"""
        code, out, err = self.t("import -", input=task_data)
        self.assertIn("Imported 4 tasks", err)

        # By default, purge does not occur.
        code, out, err = self.t("sync")
        self.assertTrue(self.exists(old_pending))
        self.assertTrue(self.exists(old_completed))
        self.assertTrue(self.exists(new_deleted))
        self.assertTrue(self.exists(old_deleted))

        # Configure purge on sync. The old_deleted task
        # should be removed.
        self.t.config("purge.on-sync", "1")
        code, out, err = self.t("sync")
        self.assertTrue(self.exists(old_pending))
        self.assertTrue(self.exists(old_completed))
        self.assertTrue(self.exists(new_deleted))
        self.assertFalse(self.exists(old_deleted))


class TestDelete(TestCase):
    def setUp(self):
        self.t = Task()

    def test_add_delete_purge(self):
        """Verify that add/delete/purge successfully purges a task"""
        self.t("add one")
        uuid = self.t("_get 1.uuid")[1].strip()

        code, out, err = self.t("1 delete", input="y\n")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t(uuid + " purge", input="y\n")
        self.assertIn("Purged 1 task.", out)

        code, out, err = self.t("uuids")
        self.assertNotIn(uuid, out)

    def test_purge_remove_deps(self):
        """Purge command removes broken dependency references"""
        self.t("add one")
        self.t("add two dep:1")
        uuid = self.t("_get 1.uuid")[1].strip()

        code, out, err = self.t("1 delete", input="y\n")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t(uuid + " purge", input="y\n")
        self.assertIn("Purged 1 task.", out)

        code, out, err = self.t("uuids")
        self.assertNotIn(uuid, out)

        dependencies = self.t("_get 1.depends")[1].strip()
        self.assertNotIn(uuid, dependencies)

    def test_purge_children(self):
        """Purge command indirectly purges child tasks"""
        self.t("add one recur:daily due:yesterday")
        uuid = self.t("_get 1.uuid")[1].strip()

        # A dummy call to report, so that recurrence tasks get generated
        self.t("list")

        code, out, err = self.t("1 delete", input="y\ny\n")
        self.assertIn("Deleted 4 tasks.", out)

        code, out, err = self.t(uuid + " purge", input="y\ny\n")
        self.assertIn("Purged 4 tasks.", out)

        code, out, err = self.t("uuids")
        self.assertEqual('\n', out)

    def test_purge_children_fail_pending(self):
        """Purge aborts if task has pending children"""
        self.t("add one recur:daily due:yesterday")
        uuid = self.t("_get 1.uuid")[1].strip()

        # A dummy call to report, so that recurrence tasks get generated
        self.t("list")

        code, out, err = self.t("1 delete", input="y\nn\n")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t.runError(uuid + " purge", input="y\n")
        # The id of the problematic task is not deterministic, as there are
        # three child tasks.
        self.assertIn("child task", err)
        self.assertIn("must be deleted before", err)

        # Check that nothing was purged
        code, out, err = self.t("count")
        self.assertEqual('4\n', out)

    def test_purge_children_fail_confirm(self):
        """Purge aborts if user does not agree with it affecting child tasks"""
        self.t("add one recur:daily due:yesterday")
        uuid = self.t("_get 1.uuid")[1].strip()

        # A dummy call to report, so that recurrence tasks get generated
        self.t("list")

        code, out, err = self.t("1 delete", input="y\ny\n")
        self.assertIn("Deleted 4 tasks.", out)

        # Do not agree with purging of the child tasks
        code, out, err = self.t.runError(uuid + " purge", input="y\nn\n")
        self.assertIn("Purge operation aborted.", err)

        # Check that nothing was purged
        code, out, err = self.t("count")
        self.assertEqual('4\n', out)

    def test_purge_children(self):
        """Purge command removes dependencies on indirectly purged tasks"""
        self.t("add one recur:daily due:yesterday")
        uuid = self.t("_get 1.uuid")[1].strip()

        # A dummy call to report, so that recurrence tasks get generated
        self.t("list")
        self.t("add two dep:4")

        # Check that the dependency is present
        dependencies = self.t("_get 5.depends")[1].strip()
        self.assertNotEqual("", dependencies)

        code, out, err = self.t("1 delete", input="y\ny\n")
        self.assertIn("Deleted 4 tasks.", out)

        code, out, err = self.t(uuid + " purge", input="y\ny\n")
        self.assertIn("Purged 4 tasks.", out)

        # Make sure we are dealing with the intended task
        description = self.t("_get 1.description")[1].strip()
        self.assertEqual("two", description)

        # Check that the dependency was removed
        dependencies = self.t("_get 1.depends")[1].strip()
        self.assertEqual("", dependencies)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
