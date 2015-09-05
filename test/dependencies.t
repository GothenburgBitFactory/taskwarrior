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


class TestDependencies(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add one")
        self.t("add two")

    def test_removing_missing_dep(self):
        """Remove a dependency that isn't there"""
        code, out, err = self.t.runError("1 modify dep:-2")
        self.assertIn("Could not delete a dependency on task 2 - not found.", err)

    def test_add_missing_dep(self):
        """Add a dependency on a missing task"""
        code, out, err = self.t.runError("1 modify dep:99")
        self.assertIn("Could not create a dependency on task 99 - not found.", err)

    def test_add_dep_to_missing_task(self):
        """Add a dependency to a missing task"""
        code, out, err = self.t.runError("99 modify dep:1")
        self.assertIn("No tasks specified.", err)

    def test_add_dep_to_missing_task(self):
        """Add a dependency to a missing task"""
        code, out, err = self.t.runError("99 modify dep:1")
        self.assertIn("No tasks specified.", err)

    def test_double_dep(self):
        """Check adding a dep twice is an error"""
        self.t("2 modify dep:1")
        code, out, err = self.t("2 modify dep:1")
        self.assertIn("Task 2 already depends on task 1.", err)

    def test_circular_1(self):
        """Check a task cannot depend on itself"""
        code, out, err = self.t.runError("1 modify dep:1")
        self.assertIn("A task cannot be dependent on itself.", err)

    def test_circular_2(self):
        """Check circular dependencies are caught, using 2 tasks"""
        self.t("2 modify dep:1")
        code, out, err = self.t.runError("1 modify dep:2")
        self.assertIn("Circular dependency detected and disallowed.", err)

    def test_circular_5(self):
        """Check circular dependencies are caught, using 5 tasks"""
        self.t("add three") 
        self.t("add four") 
        self.t("add five") 
        self.t("5 modify dep:4")
        self.t("4 modify dep:3")
        self.t("3 modify dep:2")
        self.t("2 modify dep:1")
        code, out, err = self.t.runError("1 modify dep:5")
        self.assertIn("Circular dependency detected and disallowed.", err)

    def test_dag(self):
        """Check acyclic graph support"""
        self.t("add three")
        self.t("1 modify dep:2")
        self.t("1 modify dep:3")
        self.t("2 modify dep:3")
        code, out, err = self.t("1 modify dep:2")
        self.assertNotIn("Circular dependency detected and disallowed.", err)

    def test_blocked_blocking(self):
        """Check blocked/blocking status of two tasks"""
        self.t("2 modify dep:1")

        code, out, err = self.t("_get 1.tags.BLOCKED")
        self.assertEqual("\n", out)
        code, out, err = self.t("_get 1.tags.BLOCKING")
        self.assertEqual("BLOCKING\n", out)

        code, out, err = self.t("_get 2.tags.BLOCKED")
        self.assertEqual("BLOCKED\n", out)
        code, out, err = self.t("_get 2.tags.BLOCKING")
        self.assertEqual("\n", out)

    def test_modify_multiple(self):
        """Check circular dependencies are caught, using 5 tasks"""
        self.t("add three") 
        self.t("add four") 
        self.t("add five") 
        code, out, err = self.t("1 modify dep:2,3,4")
        self.assertIn("Modified 1 task.", out)

        code, out, err = self.t("1 modify dep:5,-4")
        self.assertIn("Modified 1 task.", out)

        code, out, err = self.t("_get 3.tags.BLOCKING")
        self.assertEqual("BLOCKING\n", out)

        code, out, err = self.t("_get 4.tags.BLOCKING")
        self.assertEqual("\n", out)

    def test_done_dep(self):
        """Check that completing a task unblocks"""
        self.t("1 modify dep:2")

        code, out, err = self.t("_get 1.tags.BLOCKED")
        self.assertEqual("BLOCKED\n", out)

        code, out, err = self.t("2 done")
        self.assertIn("Unblocked 1 'one'.", out)

        code, out, err = self.t("_get 1.tags.BLOCKED")
        self.assertEqual("\n", out)

    def test_chain_repair(self):
        """Check that a broken chain is repaired"""
        self.t("add three")
        self.t("add four")
        self.t("2 modify dep:1")
        self.t("3 modify dep:2")
        self.t("4 modify dep:3")

        # 1 <-- 2 <-- 3 <-- 4  Completing 2 requires repair
        # 1 <-- 3 <-- 4
        code, out, err = self.t("2 done", input="y\n")
        self.assertIn("Would you like the dependency chain fixed?", out)

        # 1 <-- 3 <-- 4  Completing 1 requires no repair
        # 3 <-- 4
        code, out, err = self.t("1 done")
        self.assertNotIn("Would you like the dependency chain fixed?", out)

        # 3 <-- 4  Deleting 4 requires no repair
        # 3
        code, out, err = self.t("4 delete", input="y\n")
        self.assertNotIn("Would you like the dependency chain fixed?", out)
        self.assertIn("Deleted 1 task", out)

    @unittest.expectedFailure
    def test_id_range_dep(self):
        """Check that an ID range can be used for deps"""
        self.t("add three")

        # Add a range of IDs
        self.t("3 modify dep:1-2")
        code, out, err = self.t("_get 1.tags.BLOCKING")
        self.assertEqual("BLOCKING\n", out)
        code, out, err = self.t("_get 2.tag.BLOCKING")
        self.assertEqual("BLOCKING\n", out)

    def test_id_uuid_dep(self):
        """Check that IDs and UUIDs are both usable for deps"""

        # Get 2.uuid
        code, out, err = self.t("_get 2.uuid")
        uuid = out.strip()

        # Add a mix of IDs and UUID
        code, out, err = self.t("add three dep:1,%s" % uuid)
        self.assertIn("Created task 3.", out)

        # Remove a mix of IЅs and UUID
        code, out, err = self.t("3 modify dep:-1,-%s" % uuid)
        self.assertIn("Modifying task 3 'three'.", out)


# TODO - test dependency.confirmation config variable
# TODO - test undo on backing out chain gap repair
# TODO - test undo on backing out choice to not perform chain gap repair
# TODO - test blocked task completion nag
# TODO - test depend.any and depend.none report filters


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
