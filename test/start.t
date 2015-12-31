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


class TestStart(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_start_stop(self):
        """Add, start, stop a task"""
        self.t("add one")
        self.t("add two")
        code, out, err = self.t.runError("active")

        self.t("1,2 start")
        code, out, err = self.t("active")
        self.assertIn("one", out)
        self.assertIn("two", out)

        self.t("1 stop")
        code, out, err = self.t("active")
        self.assertNotIn("one", out)
        self.assertIn("two", out)

        self.t("2 stop")
        code, out, err = self.t.runError("active")

        self.t("2 done")
        code, out, err = self.t("list")
        self.assertNotIn("two", out)

    def test_journal_time(self):
        """Verify journal.time tracks state"""
        self.t.config("journal.time", "on")

        self.t("add one")
        self.t("1 start")
        code, out, err = self.t("long")
        self.assertIn("Started task", out)

        self.t("1 stop")
        code, out, err = self.t("long")
        self.assertIn("Stopped task", out)

    def test_journal_annotations(self):
        """Verify journal start/stop annotations are used"""
        self.t.config("journal.time",                  "on")
        self.t.config("journal.time.start.annotation", "Nu kör vi")
        self.t.config("journal.time.stop.annotation",  "Nu stannar vi")

        self.t("add one")
        self.t("1 start")
        code, out, err = self.t("long")
        self.assertIn("Nu kör vi", out)

        self.t("1 stop")
        code, out, err = self.t("long")
        self.assertIn("Nu stannar vi", out)


class TestActiveTaskHandling(TestCase):
    def setUp(self):
        self.t = Task()
        self.t("add one +one")

    def test_start_completed(self):
        """Completed task set to pending by start"""
        self.t("+one done")
        self.t("+one start")
        tl = self.t.export()
        self.assertEqual(tl[0]["status"], "pending")

    def test_start_deleted(self):
        """Deleted task set to pending by start"""
        self.t("+one delete", input="y\n")
        self.t("+one start")
        tl = self.t.export()
        self.assertEqual(tl[0]["status"], "pending")

    def test_start_nothing(self):
        """Verify error message when no tasks are specified"""
        code, out, err = self.t.runError ("999 start")
        self.assertIn("No tasks specified.", err)

    def test_start_started(self):
        """Verify error when starting a started task"""
        self.t("1 start")
        code, out, err = self.t.runError("1 start")
        self.assertIn("Task 1 'one' already started.", out)


class TestFeature608(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_done_stop(self):
        """608: Done should stop an active task"""
        self.t("add foo")
        self.t("1 start")
        code, out, err = self.t("export")
        self.assertIn('"start":', out)
        self.assertNotIn('"end":', out)

        self.t("1 done")
        code, out, err = self.t("export")
        self.assertNotIn('"start":', out)
        self.assertIn('"end":', out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
