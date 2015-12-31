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

class TestBug697(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    @unittest.expectedFailure
    def test_blocking_to_recurring(self):
        """697: Verify that making a blocking task into a recurring task breaks dependencies

           Bug 697: Making a blocking task recur breaks dependency.
             1. Create 2 tasks: "foo" and "bar".
             2. Give "bar" a due date.
             3. Make "foo" depend on "bar".
             4. Make "bar" recur yearly.
        """
        self.t("add one")
        self.t("add two")
        self.t("2 modify due:eom")
        self.t("1 modify depends:2")
        self.t("2 modify recur:yearly")
        self.t("list") # GC/handleRecurrence

        # The problem is that although 1 --> 2, 2 is now a recurring parent, and as 1
        # depends on the parent UUID, it is not something transferred to the child on
        # generation, because the dependency belongs with 1, not 2.

        code, out, err = self.t("_get 1.tag.BLOCKED")
        self.assertEqual("BLOCKED\n", out)

        code, out, err = self.t("_get 2.tag.BLOCKING")
        self.assertEqual("BLOCKING\n", out)

        code, out, err = self.t("_get 3.tag.BLOCKED")
        self.assertEqual("BLOCKED\n", out)


@unittest.skip("WaitingFor TW-1262")
class TestBug1262(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        cls.t('add "Buy oranges"')
        cls.t('add "Buy apples"')

        cls.DEPS = ("1", "2")
        cls.t("add dep:" + ",".join(cls.DEPS) + '"Make fruit salad!"')

    def test_dependency_contains_matches_ID(self):
        """1262: dep.contains matches task IDs"""
        # NOTE: A more robust test is needed as alternative to this
        # If it happens that the UUID doesn't contain a 1 nor a 2 the test will
        # fail, which means it's actually using the UUID.
        # Still, it passes on most cases. Which is WRONG!.
        for char in self.DEPS:
            self.t("list dep.contains:{0}".format(char))

    def test_dependency_contains_not_matches_other(self):
        """1262: dep.contains matches other characters not present in ID nor UUID"""
        for char in set(string.letters).difference(string.hexdigits):
            self.t.runError("list dep.contains:{0}".format(char))

    def test_dependency_contains_not_UUID(self):
        """1262: dep.contains matches characters in the tasks' UUIDs"""
        # Get the UUID of the task with description "Buy"
        code, out, err = self.t("uuid Buy")

        # Get only characters that show up in the UUID
        uuid = {chr for chr in out.splitlines()[0] if chr in string.hexdigits}

        for char in uuid:
            if char not in self.DEPS:
                self.t.runError("list dep.contains:{0}".format(char))


class TestFeature725(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_unbocked_feedback(self):
        """725: Verify that when a task becomes unblocked, feedback is generated"""
        self.t("add one")
        self.t("add two")
        self.t("add three")
        self.t("add four")
        self.t("1 modify depends:2,3")
        self.t("4 modify depends:1")

        # 2 does not unblock 1.
        code, out, err = self.t("2 done")
        self.assertNotIn("Unblocked", out)

        # 2 and 3 do unblock 1.
        code, out, err = self.t("3 done")
        self.assertIn("Unblocked", out)

        # 1 does unblock 4.
        code, out, er = self.t("1 delete", input="y\n")
        self.assertIn("Unblocked", out)


class Test1481(TestCase):
    def setUp(self):
        self.t = Task()
        self.t('add parent')
        self.t('add child')
        self.t('add child2')
        self.child1_uuid = self.t.export_one(2)['uuid']
        self.child2_uuid = self.t.export_one(3)['uuid']

    def test_set_dependency_on_first_completed_task(self):
        """1481: Sets dependency on task which has been just completed."""
        self.t('2 done')

        # Trigger the GC to clear up IDs
        self.t('next')

        # Set the dependency
        self.t('1 modify depends:%s' % self.child1_uuid)

    def test_set_dependency_on_second_completed_task(self):
        """
        1481: Sets dependency on task which has been completed
        before most recently completed task.
        """

        self.t('2 done')
        self.t('3 done')

        # Trigger the GC to clear up IDs
        self.t('next')

        # Set the dependencies
        self.t('1 modify depends:%s' % self.child2_uuid)

    def test_set_dependency_on_two_completed_tasks(self):
        """ 1481: Sets dependency on two most recent completed tasks. """
        self.t('2 done')
        self.t('3 done')

        # Trigger the GC to clear up IDs
        self.t('next')

        # Set the dependencies
        self.t('1 modify depends:%s,%s' % (self.child1_uuid,
                                           self.child2_uuid))


# TODO - test dependency.confirmation config variable
# TODO - test undo on backing out chain gap repair
# TODO - test undo on backing out choice to not perform chain gap repair
# TODO - test blocked task completion nag
# TODO - test depend.any and depend.none report filters


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
