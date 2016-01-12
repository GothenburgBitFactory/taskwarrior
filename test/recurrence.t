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
import re
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestRecurrenceSorting(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.asc.columns",  "id,recur,description")
        cls.t.config("report.asc.sort",     "recur+")
        cls.t.config("report.asc.filter",   "status:pending")
        cls.t.config("report.desc.columns", "id,recur,description")
        cls.t.config("report.desc.sort",    "recur-")
        cls.t.config("report.desc.filter",  "status:pending")

        cls.t("add one   due:tomorrow recur:daily")
        cls.t("add two   due:tomorrow recur:weekly")
        cls.t("add three due:tomorrow recur:3d")

    def setUp(self):
        """Executed before each test in the class"""

    def test_sort_ascending(self):
        """Verify sorting by 'recur+' is correct"""
        code, out, err = self.t("asc rc.verbose:nothing")
        self.assertRegexpMatches(out, "4\s+P1D\s+one\s+6\s+P3D\s+three\s+5\s+P7D\s+two")

    def test_sort_descending(self):
        """Verify sorting by 'recur-' is correct"""
        code, out, err = self.t("desc rc.verbose:nothing")
        self.assertRegexpMatches(out, "5\s+P7D\s+two\s+6\s+P3D\s+three\s+4\s+P1D\s+one")


class TestRecurrenceDisabled(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_reccurence_disabled(self):
        """
        Test that recurrent tasks are not being generated when recurrence is
        disabled.
        """

        self.t.config("recurrence", "no")
        self.t("add due:today recur:daily Recurrent task.")

        # Trigger GC, expect no match and therefore non-zero code
        self.t.runError("list")

        # Check that no task has been generated.
        code, out, err = self.t("task count")
        self.assertEqual("0", out.strip())


class TestRecurrenceLimit(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_recurrence_limit(self):
        """Verify that rc.recurrence.limit is obeyed"""
        self.t("add one due:tomorrow recur:weekly")
        code, out, err = self.t("list")
        self.assertEqual(out.count("one"), 1)

        code, out, err = self.t("list rc.recurrence.limit:4")
        self.assertEqual(out.count("one"), 4)


class TestRecurrenceWeekdays(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_recurrence_weekdays(self):
        """Verify that 'recur:weekdays' skips weekends"""

        # Add a 'recur:weekdays' task due on a friday, which forces the next
        # instance to be monday, thereby skipping the weekend.
        self.t("add due:friday recur:weekdays one")

        # Get the original due date as a julian date.
        self.t("list")  # GC/handleRecurrence
        code, friday, err = self.t("_get 2.due.julian")

        # Generate the second instance, obtain due date.
        self.t("list rc.recurrence.limit:2")  # GC/handleRecurrence
        code, monday, err = self.t("_get 3.due.julian")

        # The due dates should be Friday and Monday, three days apart,
        # having skipped the weekend.
        # Note: On daylight savings in the fall, this '3' becomes '2.9583'.
        self.assertTrue(int(monday.strip()) - int(friday.strip()) >= 2)


class TestRecurrenceUntil(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_recurrence_until(self):
        """Verify that an 'until' date terminates recurrence"""

        self.t("add one due:now+1minute recur:PT1H until:now+125minutes")
        code, out, err = self.t("list rc.verbose:nothing")
        self.assertEqual(out.count("one"), 1)

        # All three expected tasks are shown:
        #   - PT1M
        #   - PT61M
        #   - PT121M
        #   - Nothing after PT125M
        self.t.faketime("+3h")
        code, out, err = self.t("list rc.verbose:nothing")
        self.assertEqual(out.count("one"), 3)

        # This test verifies that 'until' propagates to the instances, which
        # was not the original intention, but has been this way for a a while.
        # Given that the original decision was arbitrary, this is no worse.
        # We shall preserve recent behavior.
        self.t.faketime("+24h")
        code, out, err = self.t("( status:pending or status:recurring ) count")
        self.assertEqual("0\n", out)
        self.assertIn("Task 2 'one' expired and was deleted.", err)
        self.assertIn("Task 3 'one' expired and was deleted.", err)
        self.assertIn("Task 4 'one' expired and was deleted.", err)


class TestRecurrenceTasks(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add simple")
        self.t("add complex due:today recur:daily")

    def test_change_propagation(self):
        """Verify that changes (modify, delete) are propagated correctly"""

        # List tasks to generate child tasks.  Should result in:
        #   1 simple
        #   3 complex
        #   4 complex
        code, out, err = self.t("minimal rc.verbose:nothing")
        self.assertRegexpMatches(out, "1\s+simple")
        self.assertRegexpMatches(out, "3\s+complex")
        self.assertRegexpMatches(out, "4\s+complex")

        # Modify a child task and do not propagate the change.
        self.t("3 modify complex2", input="n\n")
        code, out, err = self.t("_get 3.description")
        self.assertEqual("complex2\n", out)
        code, out, err = self.t("_get 4.description")
        self.assertEqual("complex\n", out)

        # Modify a child task and propagate the change.
        self.t("3 modify complex3", input="y\n")
        code, out, err = self.t("_get 3.description")
        self.assertEqual("complex3\n", out)
        code, out, err = self.t("_get 4.description")
        self.assertEqual("complex3\n", out)

        # Delete a child task, do not propagate.
        code, out, err = self.t("3 delete", input="y\n")
        self.assertIn("Deleted 1 task.", out)

        # Delete a child task, propagate.
        self.t("minimal")
        code, out, err = self.t("3 delete", input="y\n")
        self.assertIn("Deleted 1 task.", out)

        # Check for duplicate UUIDs.
        code, out, err = self.t("diag")
        self.assertIn("No duplicates found", out)


class TestBug972(TestCase):
    def setUp(self):
        """972: Executed before each test in the class"""
        self.t = Task()

    def test_interpretation_of_seven(self):
        """Bug 972: A recurrence period of "7" is interpreted as "7s", not "7d"
           as intended.
        """
        code, out, err = self.t.runError("add one due:now recur:2")
        self.assertIn("The duration value '2' is not supported.", err)


class TestDeletionRecurrence(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_delete_parent(self):
        """Delete a parent with child tasks"""
        self.t("add one due:eom recur:daily")
        self.t("list") # GC/handleRecurrence
        code, out, err = self.t("1 delete", input="y\ny\n")
        self.assertIn("Deleted 2 tasks.", out)

        code, out, err = self.t.runError("list")
        self.assertIn("No matches.", err)

    def test_delete_child_with_siblings(self):
        """Delete a child with sibling tasks"""
        self.t("add one due:eom recur:daily")
        self.t("list rc.recurrence.limit:5")
        code, out, err = self.t("list rc.verbose:nothing") # GC/handleRecurrence
        self.assertEqual(out.count("one"), 5)

        code, out, err = self.t("2 delete", input="y\ny\n")
        self.assertIn("Deleted 5 tasks.", out)


class TestAppendPrependRecurrence(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_append_propagate(self):
        """Append and propagate"""
        self.t("add one due:eom recur:daily")
        self.t("list rc.recurrence.limit:2") # GC/handleRecurrence

        code, out, err = self.t("2 append APP", input="y\n")
        self.assertIn("Appended 2 tasks.", out)

    def test_prepend_propagate(self):
        """Prepend and propagate"""
        self.t("add one due:eom recur:daily")
        self.t("list rc.recurrence.limit:2") # GC/handleRecurrence

        code, out, err = self.t("2 prepend PRE", input="y\n")
        self.assertIn("Prepended 2 tasks.", out)


class TestNoDueDate(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_no_due_date(self):
        """Look for an error when adding a recurring task with no due date"""
        code, out, err = self.t.runError("add foo recur:daily")
        self.assertIn("A recurring task must also have a 'due' date.", err)


class TestBug932(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("report.id_pri_proj.columns", "id,priority,project")
        self.t.config("report.id_pri_proj.labels", "ID,P,Proj")

    def test_modify_due_propagate(self):
        """932: Verify due date modifications propagate"""

        # add a recurring task with multiple child tasks
        # - modify a child task and test for propagation
        # - modify the parent task and test for propagation
        self.t("add R due:yesterday recur:daily")
        self.t("list")  # GC/handleRecurrence

        self.t("2 modify project:P", input="y\n")
        code, out, err = self.t("id_pri_proj")
        self.assertIn("2 P", out)
        self.assertIn("3 P", out)
        self.assertIn("4 P", out)

        self.t("1 modify priority:H", input="y\n")
        code, out, err = self.t("id_pri_proj")
        self.assertIn("2 H P", out)
        self.assertIn("3 H P", out)
        self.assertIn("4 H P", out)


class TestBug955(TestCase):
    def setUp(self):
        self.t = Task()

        self.t("add foo due:now recur:1day")
        code, out, err = self.t("ls")
        self.assertRegexpMatches(out, re.compile("^2 tasks", re.MULTILINE))

    def test_no_prompt_for_parent_on_child_delete_confirmation_off(self):
        """955: Deleting a child of a recurring task prompts for parent deletion (confirmation:off)

        Previously bug.955.t
        """

        # With confirmation:off the first "n\n" has no effect.
        self.t.config("confirmation", "off")
        code, out, err = self.t("2 delete", input="n\ny\n")
        self.assertIn("Deleting task 2", out)
        self.assertIn("Deleted 1 task", out)

        # 1 task remains.
        code, out, err = self.t("ls")
        self.assertRegexpMatches(out, re.compile("^1 task", re.MULTILINE))

        code, out, err = self.t("2 delete", input="y\ny\n")
        self.assertIn("Deleting task 2", out)
        self.assertIn("Deleted 1 task", out)

        code, out, err = self.t.runError("all status:recurring")
        self.assertIn("No matches", err)

        code, out, err = self.t.runError("ls")
        self.assertIn("No matches", err)

    def test_no_prompt_for_parent_on_child_delete_confirmation_on(self):
        """Deleting a child of a recurring task prompts for parent deletion (confirmation:on)

        Previously bug.955.t
        """

        # With confirmation:on the first "n\n" is obeyed.
        self.t.config("confirmation", "on")
        code, out, err = self.t.runError("2 delete", input="n\ny\n")
        self.assertIn("Deleted 0 tasks.", out)

        # 2 tasks intact.
        code, out, err = self.t("ls")
        self.assertRegexpMatches(out, re.compile("^2 tasks", re.MULTILINE))

        code, out, err = self.t("2 delete", input="y\ny\n")
        self.assertIn("Deleted 2 tasks", out)

        code, out, err = self.t.runError("all status:recurring")
        self.assertIn("No matches", err) 

        code, out, err = self.t.runError("ls")
        self.assertIn("No matches", err)

class TestUpgradeToRecurring(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_upgrade(self):
        """Upgrade task to recurring"""
        self.t("add foo")
        self.t("1 modify due:tomorrow recur:weekly")
        code, out, err = self.t("_get 1.status")
        self.assertEqual("recurring\n", out)

    def test_failed_upgrade(self):
        """Attempt an upgrade, but omit the due date"""
        self.t("add foo")
        code, out, err = self.t.runError("1 modify recur:weekly")
        self.assertIn("You cannot specify a recurring task without a due date.", err)

class TestRecurrenceNotification(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_notification(self):
        """Test notification on task creation"""

        self.t("add foo due:eow recur:daily")

        code, out, err = self.t("list")
        self.assertIn("Creating recurring task instance 'foo'", err)

        code, out, err = self.t("list")
        self.assertNotIn("Creating recurring task instance 'foo'", err)

class BaseTestBug360(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add foo due:tomorrow recur:daily")
        # This command forces a handleRecurrence() call to generate synthetic tasks.
        self.t("ls")

class TestBug360RemovalError(BaseTestBug360):
    def test_modify_recursive_project(self):
        """360: Modifying a recursive task by adding project: also modifies parent
        """
        code, out, err = self.t("1 modify project:bar", input="y\n")

        expected = "Modified 2 tasks."
        self.assertIn(expected, out)
        expected = "You cannot remove the recurrence from a recurring task."
        self.assertNotIn(expected, err)

    def test_cannot_remove_recurrence(self):
        """360: Cannot remove recurrence from recurring task
        """
        # TODO Removing recur: from a recurring task should also remove imask
        # and parent.

        code, out, err = self.t.runError("2 modify recur:")
        # Expected non zero exit-code
        self.assertEqual(code, 2)

        expected = "You cannot remove the recurrence from a recurring task."
        self.assertIn(expected, err)

    def test_cannot_remove_due_date(self):
        """360: Cannot remove due date from recurring task
        """
        # TODO Removing due: from a recurring task should also remove recur,
        # imask and parent
        code, out, err = self.t.runError("2 modify due:")
        # Expected non zero exit-code
        self.assertEqual(code, 2)

        expected = "You cannot remove the due date from a recurring task."
        self.assertIn(expected, err)


class TestBug360AllowedChanges(BaseTestBug360):
    def setUp(self):
        """Executed before each test in the class"""
        # Also do setUp from BaseTestBug360
        super(TestBug360AllowedChanges, self).setUp()

        self.t("add nonrecurring due:today")

    def test_allow_modify_due_in_nonrecurring(self):
        """360: Allow modifying due date in non recurring task"""
        # Retrieve the id of the non recurring task
        code, out, err = self.t("ls")

        expected = "2 tasks"
        self.assertIn(expected, out)

        # NOTE: raw python string r"" avoids having to escape backslashes
        id = re.search(r"(\d+)\s.+\snonrecurring", out).group(1)

        code, out, err = self.t((id, "modify", "due:"))

        expected = "Modified 1 task."
        self.assertIn(expected, out)
        expected = "You cannot remove the due date from a recurring task."
        self.assertNotIn(expected, err)

        # Make sure no duplicate tasks were created
        code, out, err = self.t.diag()
        expected = "No duplicates found"
        self.assertIn(expected, out)

class TestBug649(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_delete_recurring_immediately(self):
        """649: Verify that recurring tasks cannot be immediately marked completed"""
        self.t("add one due:3d recur:1week")
        code, out, err = self.t.runError("1 done")
        self.assertIn("is neither pending nor waiting", out)
        self.assertNotIn("Completed 1", out)

class TestBugC001(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_id_increment(self):
        """C001: Verify that entering two consecutive recurring tasks increments reported ID"""
        code, out, err = self.t("add one due:tomorrow recur:daily")
        code, out, err = self.t("add two due:tomorrow recur:daily")

class TestBug839(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_recurrence_value_mapping(self):
        """839: Verify that importing a legacy recurrence value is ok"""
        json = '{"description":"one","due":"1437579505","recur":"1m","status":"recurring","uuid":"ebeeab00-ccf8-464b-8b58-f7f2d606edfb"}'
        self.t("import -", input=json)

        code, out, err = self.t("list")
        self.assertIn("one", out)
        self.assertNotIn("The recurrence value '1m' is not valid.", out)


class TestPeriod(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_recurrence_periods(self):
       """Verify recurrence period special-case support

       Date getNextRecurrence (Date& current, std::string& period)

       Confirmed:
         getNextRecurrence  convertDuration
         -----------------  ---------------
                            daily
                            day
                            weekly
                            sennight
                            biweekly
                            fortnight
         monthly            monthly
         quarterly          quarterly
         semiannual         semiannual
         bimonthly          bimonthly
         biannual           biannual
         biyearly           biyearly
                            annual
                            yearly
         *m                 *m
         *q                 *q
                            *d
                            *w
                            *y
       """

       self.t("add daily due:tomorrow recur:daily")
       self.t("add 1day due:tomorrow recur:1day")
       self.t("add weekly due:tomorrow recur:weekly")
       self.t("add 1sennight due:tomorrow recur:1sennight")
       self.t("add biweekly due:tomorrow recur:biweekly")
       self.t("add fortnight due:tomorrow recur:fortnight")
       self.t("add monthly due:tomorrow recur:monthly")
       self.t("add quarterly due:tomorrow recur:quarterly")
       self.t("add semiannual due:tomorrow recur:semiannual")
       self.t("add bimonthly due:tomorrow recur:bimonthly")
       self.t("add biannual due:tomorrow recur:biannual")
       self.t("add biyearly due:tomorrow recur:biyearly")
       self.t("add annual due:tomorrow recur:annual")
       self.t("add yearly due:tomorrow recur:yearly")
       self.t("add 2d due:tomorrow recur:2d")
       self.t("add 2w due:tomorrow recur:2w")
       self.t("add 2mo due:tomorrow recur:2mo")
       self.t("add 2q due:tomorrow recur:2q")
       self.t("add 2y due:tomorrow recur:2y")

       # Verify that the recurring task instances were created.  One of each.
       code, out, err = self.t("list")
       self.assertIn(" daily ",      out);
       self.assertIn(" 1day ",       out);
       self.assertIn(" weekly ",     out);
       self.assertIn(" 1sennight ",  out);
       self.assertIn(" biweekly ",   out);
       self.assertIn(" fortnight ",  out);
       self.assertIn(" monthly ",    out);
       self.assertIn(" quarterly ",  out);
       self.assertIn(" semiannual ", out);
       self.assertIn(" bimonthly ",  out);
       self.assertIn(" biannual ",   out);
       self.assertIn(" biyearly ",   out);
       self.assertIn(" annual ",     out);
       self.assertIn(" yearly ",     out);
       self.assertIn(" 2d ",         out);
       self.assertIn(" 2w ",         out);
       self.assertIn(" 2mo ",        out);
       self.assertIn(" 2q ",         out);
       self.assertIn(" 2y ",         out);

       # Duplicate check
       code, out, err = self.t("diag")
       self.assertIn("No duplicates found", out)


class TestBugAnnual(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_annual_creep(self):
        """Verify 'annual' recurring tasks don't creep"""
        self.t.config("dateformat",            "YMD")
        self.t.config("report.annual.labels",  "ID,Due")
        self.t.config("report.annual.columns", "id,due")
        self.t.config("report.annual.filter",  "status:pending")
        self.t.config("report.annual.sort",    "due+")

        # If a task is added with a due date ten years ago, with an annual recurrence,
        # then the synthetic tasks in between then and now have a due date that creeps.
        #
        # ID Due        Description
        # -- ---------- -----------
        #  4 1/1/2002   foo
        #  5 1/1/2003   foo
        #  6 1/1/2004   foo
        #  7 1/1/2005   foo
        #  8 1/1/2006   foo
        #  9 1/1/2007   foo
        # 10 1/1/2008   foo
        # 11 1/1/2009   foo
        # 12 1/1/2010   foo
        #  2 1/1/2000   foo
        #  3 1/1/2001   foo

        self.t("add foo due:20000101 recur:annual until:20150101")
        code, out, err = self.t("annual")
        self.assertIn(" 2 20000101", out)
        self.assertIn(" 3 20010101", out)
        self.assertIn(" 4 20020101", out)
        self.assertIn(" 5 20030101", out)
        self.assertIn(" 6 20040101", out)
        self.assertIn(" 7 20050101", out)
        self.assertIn(" 8 20060101", out)
        self.assertIn(" 9 20070101", out)
        self.assertIn("10 20080101", out)
        self.assertIn("11 20090101", out)
        self.assertIn("12 20100101", out)
        self.assertIn("13 20110101", out)
        self.assertIn("14 20120101", out)
        self.assertIn("15 20130101", out)
        self.assertIn("16 20140101", out)
        self.assertIn("17 20150101", out)


# TODO Wait a recurring task
# TODO Downgrade a recurring task to a regular task
# TODO Duplicate a recurring child task
# TODO Duplicate a recurring parent task


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
