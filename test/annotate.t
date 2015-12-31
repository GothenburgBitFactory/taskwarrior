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


class TestAnnotate(TestCase):
    def setUp(self):
        self.t = Task()

        # ID Description
        # -- -------------------------------
        #  1 one
        #    3/24/2009 foo1
        #    3/24/2009 foo2
        #    3/24/2009 foo3
        #  2 two
        #    3/24/2009 bar1
        #    3/24/2009 bar2
        #  3 three
        #    3/24/2009 baz1
        #  4 four
        #
        # 4 tasks

        self.t("add one")
        self.t("add two")
        self.t("add three")
        self.t("add four")
        self.t("1 annotate foo1")
        self.t("1 annotate foo2")
        self.t("1 annotate foo3")
        self.t("2 annotate bar1")
        self.t("2 annotate bar2")
        self.t("3 annotate baz1")

    def assertTasksExist(self, out):
        self.assertIn("1 one", out)
        self.assertIn("2 two", out)
        self.assertIn("3 three", out)
        self.assertIn("4 four", out)
        self.assertIn("4 tasks", out)

    def test_annotate(self):
        """Testing annotations in reports"""

        # NOTE: Use 'rrr' to guarantee a unique report name.  Using 'r'
        # conflicts with 'recurring'.
        self.t.config("report.rrr.description", "rrr")
        self.t.config("report.rrr.columns",     "id,description")
        self.t.config("report.rrr.sort",        "id+")
        self.t.config("dateformat",             "m/d/Y")
        self.t.config("color",                  "off")

        code, out, err = self.t("rrr")

        self.assertTasksExist(out)

        self.assertRegexpMatches(out, "one\n.+\d{1,2}/\d{1,2}/\d{4}\s+foo1",
                                 msg='full - first  annotation task 1')
        self.assertRegexpMatches(out, "foo1\n.+\d{1,2}/\d{1,2}/\d{4}\s+foo2",
                                 msg='full - first  annotation task 1')
        self.assertRegexpMatches(out, "foo2\n.+\d{1,2}/\d{1,2}/\d{4}\s+foo3",
                                 msg='full - first  annotation task 1')
        self.assertRegexpMatches(out, "two\n.+\d{1,2}/\d{1,2}/\d{4}\s+bar1",
                                 msg='full - first  annotation task 1')
        self.assertRegexpMatches(out, "bar1\n.+\d{1,2}/\d{1,2}/\d{4}\s+bar2",
                                 msg='full - first  annotation task 1')
        self.assertRegexpMatches(out, "three\n.+\d{1,2}/\d{1,2}/\d{4}\s+baz1",
                                 msg='full - first  annotation task 1')

    def test_annotate_dateformat(self):
        """Testing annotations in reports using dateformat.annotation"""

        # NOTE: Use 'rrr' to guarantee a unique report name.  Using 'r'
        # conflicts with 'recurring'.
        self.t.config("report.rrr.description", "rrr")
        self.t.config("report.rrr.columns",     "id,description")
        self.t.config("report.rrr.sort",        "id+")
        self.t.config("dateformat.annotation",  "yMD HNS")

        code, out, err = self.t("rrr")

        self.assertTasksExist(out)

        self.assertRegexpMatches(out, "one\n.+\d{1,6}\s+\d{1,6}\s+foo1",
                                 msg="dateformat - first  annotation task 1")
        self.assertRegexpMatches(out, "foo1\n.+\d{1,6}\s+\d{1,6}\s+foo2",
                                 msg="dateformat - second  annotation task 1")
        self.assertRegexpMatches(out, "foo2\n.+\d{1,6}\s+\d{1,6}\s+foo3",
                                 msg="dateformat - third  annotation task 1")
        self.assertRegexpMatches(out, "two\n.+\d{1,6}\s+\d{1,6}\s+bar1",
                                 msg="dateformat - first  annotation task 2")
        self.assertRegexpMatches(out, "bar1\n.+\d{1,6}\s+\d{1,6}\s+bar2",
                                 msg="dateformat - second  annotation task 2")
        self.assertRegexpMatches(out, "three\n.+\d{1,6}\s+\d{1,6}\s+baz1",
                                 msg="dateformat - first  annotation task 3")

class TestAnnotationPropagation(TestCase):
    def setUp(self):
        self.t = Task()

    def test_annotate_nothing(self):
        """Test that an error is produced when annotating no tasks"""
        code, out, err = self.t.runError("999 annotate no way")
        self.assertIn("No tasks specified.", err)

    def test_annotate_recurring(self):
        """Test propagation of annotation to recurring siblings"""
        self.t("add foo due:eom recur:weekly")
        self.t("list") # GC/handleRecurrence
        self.t("2 annotate bar", input="y\n")
        code, out, err = self.t("all rc.verbose:nothing")

        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertEqual("bar\n", out)

        code, out, err = self.t("_get 2.annotations.1.description")
        self.assertEqual("bar\n", out)


class TestAnnotation(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("confirmation", "yes")

    def test_blank_annotation(self):
        """Verify blank annotations are prevented"""
        self.t("add foo")

        code, out, err = self.t.runError("1 annotate")
        self.assertIn("Additional text must be provided", err)

    def test_filterless_annotate_decline(self):
        """Verify filterless annotation is trapped, declined"""
        self.t("add foo")

        code, out, err = self.t.runError("annotate bar", input="no\n")
        self.assertIn("Command prevented from running", err)
        self.assertNotIn("Command prevented from running", out)

    def test_filterless_annotate(self):
        """Verify filterless annotation is trapped, overridden"""
        self.t("add foo")
        code, out, err = self.t("annotate bar", input="yes\n")

        self.assertNotIn("Command prevented from running", err)
        self.assertNotIn("Command prevented from running", out)
        self.assertIn("Annotated 1 task", out)


class TestBug495(TestCase):
    def setUp(self):
        self.t = Task()

    def test_double_hyphen_annotation(self):
        """double hyphen mishandled for annotations"""
        # NOTE: originally Bug #495
        self.t("add foo")
        self.t("1 annotate This -- is -- a -- test")

        # Double hyphens preserved except the first ones
        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertEqual("This is -- a -- test\n", out)

class TestBug694(TestCase):
    def setUp(self):
        self.t = Task()

    def test_annotation_with_due(self):
        """Add an annotation as well as a due date"""
        self.t("add one")
        self.t("1 annotate two due:today")

        code, out, err = self.t("rc.journal.info:off 1 info")
        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertIn("Due", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
