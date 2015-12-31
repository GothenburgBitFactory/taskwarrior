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

from basetest import Task, TestCase, Taskd, ServerTestCase


class TestDefaultProject(TestCase):
    """Bug 1023: rc.default.project gets applied during modify, and should not
    """
    def setUp(self):
        self.t = Task()

    def set_default_project(self):
        self.default_project = "HOMEPROJ"
        self.t.config("default.project", self.default_project)

    def test_with_project(self):
        """default.project not applied when specified nor on attribute removal
        """
        self.set_default_project()

        self.t("add foobar project:garden")
        code, out, err = self.t("1 info")

        self.assertIn("foobar", out)

        expected = "Project\s+garden"
        self.assertRegexpMatches(out, expected)

        self.t("1 modify project:")
        code, out, err = self.t("1 info")

        self.assertIn("foobar", out)
        self.assertNotRegexpMatches(out, expected)

        notexpected = "Project\s+" + self.default_project
        self.assertNotRegexpMatches(out, notexpected)

    def test_without_project(self):
        """default.project applied when no project is specified"""
        self.set_default_project()

        self.t("add foobar")
        code, out, err = self.t("1 info")

        self.assertIn("foobar", out)

        expected = "Project\s+" + self.default_project
        self.assertRegexpMatches(out, expected)

    def test_default_project_inline_override(self):
        """no project applied when default.project is overridden"""
        self.set_default_project()

        self.t("add foobar rc.default.project=")
        code, out, err = self.t("1", "info")

        self.assertIn("foobar", out)
        self.assertNotIn("Project", out)

    def test_without_default_project(self):
        """no project applied when default.project is blank"""
        self.t("add foobar")
        code, out, err = self.t("1 info")

        self.assertIn("foobar", out)
        self.assertNotIn("Project", out)

    def test_modify_default_project(self):
        """default.project is not applied when modifying a task"""
        self.t("add foobar")
        code, out, err = self.t("1 info")

        self.assertIn("foobar", out)
        self.assertNotIn("Project", out)

        self.set_default_project()

        self.t("1 modify +tag")
        code, out, err = self.t("1", "info")
        self.assertNotIn("Project", out)

    def test_annotate_default_project(self):
        """default.project is not applied when annotating a task"""
        self.t("add foobar")
        code, out, err = self.t("1 info")

        self.assertIn("foobar", out)
        self.assertNotIn("Project", out)

        self.set_default_project()

        self.t("1 annotate Hello")
        code, out, err = self.t("1 info")

        expected = "Description\s+foobar\n[0-9-: ]+ Hello"
        self.assertRegexpMatches(out, expected)
        self.assertNotIn("Project", out)

    def test_time_default_project(self):
        """default.project is not applied when start/stop'ing a task"""
        # Allow keeping track of time spent on task
        self.t.config("journal.time", "yes")

        self.t("add foobar")
        code, out, err = self.t("1 info")

        self.assertIn("foobar", out)
        self.assertNotIn("Project", out)

        self.set_default_project()

        self.t("1 start")
        self.t("1 stop")
        code, out, err = self.t("1", "info")

        self.assertIn("foobar", out)
        self.assertNotIn("Project", out)

    def test_recurring_parent_default_project(self):
        """default.project is applied on recurring parent tasks"""
        self.set_default_project()

        DESC = "foobar"
        self.t(('add', 'recur:daily', 'due:today', DESC))
        self.t()  # Ensure creation of recurring children
        code, out, err = self.t("1 info")

        self.assertIn(DESC, out)
        self.assertRegexpMatches(out, "Status\s+Recurring")  # is a parent task
        self.assertIn(self.default_project, out)

        self.t.faketime("+1d")

        self.t()  # Ensure creation of recurring children
        # Try to figure out the ID of last created task
        code, out, err = self.t("count")

        # Will fail if some other message is printed as part of "count"
        id = out.split()[-1]

        try:
            id = int(id)
        except ValueError:
            raise ValueError("Unexpected output when running 'task count', "
                             "expected int, got '{0}'".format(id))
        else:
            # parent task is not considered when counting
            id = str(id + 1)

        code, out, err = self.t(id, "info")

        self.assertIn(DESC, out)
        self.assertIn("Parent task", out)  # is a child task
        self.assertIn(self.default_project, out)

    def test_recurring_default_project(self):
        """no project is applied on recurring tasks"""
        # NOTE - reported on TW-1279
        DESC = "foobar"
        self.t(('add', 'recur:daily', 'due:today', DESC))
        code, out, err = self.t()

        self.assertIn(DESC, out)
        self.assertNotIn("Project", out)

        self.set_default_project()

        self.t.faketime("+1d")

        code, out, err = self.t()

        self.assertIn(DESC, out)
        self.assertNotIn("Project", out)

    def test_recurring_with_project_and_default_project(self):
        """default.project is not applied to children if parent has a project
        """
        # NOTE - reported on TW-1279
        self.set_default_project()

        DESC = "foobar"
        self.t(('add', 'recur:daily', 'due:today', 'project:HELLO', DESC))
        code, out, err = self.t()

        self.assertIn(DESC, out)
        self.assertIn("HELLO", out)

        self.t.faketime("+1d")

        code, out, err = self.t()

        self.assertIn("foobar", out)
        self.assertIn("HELLO", out)
        self.assertNotIn(self.default_project, out)


class ServerTestDefaultProject(ServerTestCase):
    @classmethod
    def setUpClass(cls):
        cls.taskd = Taskd()
        # This takes a while...
        cls.taskd.start()

    def setUp(self):
        self.t1 = Task(taskd=self.taskd)
        self.t2 = Task(taskd=self.taskd)
        self.t3 = Task(taskd=self.taskd)

    def test_default_project_sync(self):
        """default.project is not applied to projectless tasks during sync"""
        # NOTE - reported on TW-1287
        desc = "Testing task"
        self.t1(("add", desc))
        self.t1("sync")

        code, out, err = self.t1()

        self.assertIn(desc, out)

        # Testing scenario - default.project is applied on task arrival
        proj2 = "Client2"
        self.t2.config("default.project", proj2)
        self.t2("sync")

        code, out, err = self.t2()
        self.assertIn(desc, out)
        self.assertNotIn(proj2, out)

        self.t2("sync")

        # Testing scenario - default.project is applied on task delivery
        proj3 = "Client3"
        self.t3.config("default.project", proj3)
        self.t3("sync")

        code, out, err = self.t3()
        self.assertIn(desc, out)
        self.assertNotIn(proj2, out)
        self.assertNotIn(proj3, out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
