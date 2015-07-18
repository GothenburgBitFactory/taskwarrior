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


class TestDescriptionFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,description")
        cls.t.config("verbose",            "nothing")

        cls.t("add zero")
        cls.t("add one long description to exceed a certain string size")
        cls.t("2 annotate annotation")

    def setUp(self):
        """Executed before each test in the class"""

    def test_description_combined(self):
        """Verify formatting of 'description.combined' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,description.combined")
        self.assertIn("one long description to exceed a certain string size", out)
        self.assertRegexpMatches(out, r"\d{4}-\d{2}-\d{2} annotation")
        self.assertNotIn("[1]", out)

    def test_description_desc(self):
        """Verify formatting of 'description.desc' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,description.desc")
        self.assertIn("one long description to exceed a certain string size", out)
        self.assertNotIn("[1]", out)

    def test_description_oneline(self):
        """Verify formatting of 'description.oneline' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,description.oneline")
        self.assertRegexpMatches(out, r"one long description to exceed a certain string size \d{4}-\d{2}-\d{2}")
        self.assertIn("annotation", out)
        self.assertNotIn("[1]", out)

    def test_description_truncated(self):
        """Verify formatting of 'description.truncated' column"""
        code, out, err = self.t("xxx rc.detection:off rc.defaultwidth:40 rc.report.xxx.columns:id,description.truncated")
        self.assertIn("exceed a c...", out)
        self.assertNotIn("annotation", out)
        self.assertNotIn("[1]", out)

    def test_description_count(self):
        """Verify formatting of 'description.count' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,description.count")
        self.assertIn("size [1]", out)
        self.assertNotIn("annotation", out)

    def test_description_truncated_count(self):
        """Verify formatting of 'description.truncated_count' column"""
        code, out, err = self.t("xxx rc.detection:off rc.defaultwidth:40 rc.report.xxx.columns:id,description.truncated_count")
        self.assertIn("exceed... [1]", out)
        self.assertNotIn("annotation", out)

class TestUUIDFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,uuid")
        cls.t.config("verbose",            "nothing")

        cls.t("add zero")
        code, out, err = cls.t("_get 1.uuid")
        cls.uuid = out.strip()

    def setUp(self):
        """Executed before each test in the class"""

    def test_uuid_long(self):
        """Verify formatting of 'uuid.long' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,uuid.long")
        self.assertIn(self.uuid, out)

    def test_uuid_short(self):
        """Verify formatting of 'uuid.short' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,uuid.short")
        self.assertIn(self.uuid[:7], out)

class TestUrgencyFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,urgency")
        cls.t.config("verbose",            "nothing")

        cls.t("add one project:A due:yesterday +tag")

    def setUp(self):
        """Executed before each test in the class"""

    def test_urgency_real(self):
        """Verify formatting of 'urgency.real' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,urgency.real")
        self.assertIn("11.", out)

    def test_urgency_integer(self):
        """Verify formatting of 'urgency.integer' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,urgency.integer")
        self.assertIn("11", out)
        self.assertNotIn("11.", out)

class TestIDFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id")
        cls.t.config("verbose",            "nothing")

        cls.t("add zero")

    def setUp(self):
        """Executed before each test in the class"""

    def test_id_number(self):
        """Verify formatting of 'id.number' column"""
        code, out, err = self.t("xxx ")
        code, out, err = self.t("xxx rc.report.xxx.columns:id.number")
        self.assertEqual(" 1\n", out)

class TestStatusFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,status")
        cls.t.config("verbose",            "nothing")

        cls.t("add zero")
        cls.t("add one")
        cls.t("2 delete")
        cls.t("log two")
        cls.t("add three due:eom recur:weekly")
        cls.t("add four wait:eom")
        cls.t("list")

    def setUp(self):
        """Executed before each test in the class"""

    def test_status_short(self):
        """Verify formatting of 'status.short' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,status.short")
        self.assertIn(" 1 P", out)
        self.assertIn(" 2 R", out)
        self.assertIn(" 3 W", out)
        self.assertIn(" 4 P", out)
        self.assertIn(" - D", out)
        self.assertIn(" - C", out)

    def test_status_long(self):
        """Verify formatting of 'status.long' column"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,status.long")
        self.assertIn(" 1 Pending", out)
        self.assertIn(" 2 Recurring", out)
        self.assertIn(" 3 Waiting", out)
        self.assertIn(" 4 Pending", out)
        self.assertIn(" - Deleted", out)
        self.assertIn(" - Completed", out)

class TestRecurringAttributeFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id")
        cls.t.config("verbose",            "nothing")

        cls.t("add one due:eoy recur:monthly")
        cls.t("list")

    def setUp(self):
        """Executed before each test in the class"""

    def test_recurrence_formats_short(self):
        """Verify formatting of assorted recurrence columns"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,status,due,recur.indicator,mask,imask,parent.short")
        self.assertRegexpMatches(out, "1\sRecurring\s+\d{4}-\d{2}-\d{2}\s+R\s+-")
        self.assertRegexpMatches(out, "2\sPending\s+\d{4}-\d{2}-\d{2}\s+R\s+0\s+[0-9a-fA-F]{8}")

    def test_recurrence_formats_long(self):
        """Verify formatting of assorted recurrence columns"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,status,due,recur.duration,mask,imask,parent.long")
        self.assertRegexpMatches(out, "1\sRecurring\s+\d{4}-\d{2}-\d{2}\s+P1M\s+-")
        self.assertRegexpMatches(out, "2\sPending\s+\d{4}-\d{2}-\d{2}\s+P1M\s+0\s+[0-9a-fA-F-]{36}")


        """
depends     list*             1 2 10
            count             [3]
            indicator         D

due         formatted*        2min
            julian            2457221.33061
            epoch
            iso
            age
            remaining
            countdown

priority    default*
            indicator

project     full*             home.garden
            parent            home
            indented            home.garden

start       active*           ✓

tags        list*             home @chore next
            indicator         +
            count             [2]
        """

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
