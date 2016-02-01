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

    def test_description_format_unrecognized(self):
        """Verify descriptionuuid.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,description.donkey")
        self.assertEqual(err, "Unrecognized column format 'description.donkey'\n")


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

    def test_uuid_format_unrecognized(self):
        """Verify uuid.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,uuid.donkey")
        self.assertEqual(err, "Unrecognized column format 'uuid.donkey'\n")


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

    def test_urgency_format_unrecognized(self):
        """Verify urgency.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,urgency.donkey")
        self.assertEqual(err, "Unrecognized column format 'urgency.donkey'\n")


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

    def test_id_format_unrecognized(self):
        """Verify id.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id.donkey")
        self.assertEqual(err, "Unrecognized column format 'id.donkey'\n")


class TestStatusFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,status")
        cls.t.config("verbose",            "nothing")

        cls.t("add zero")
        cls.t("add one")
        cls.t("2 delete", input="y\n")
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

    def test_status_format_unrecognized(self):
        """Verify status.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,status.donkey")
        self.assertEqual(err, "Unrecognized column format 'status.donkey'\n")


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
        """Verify formatting of assorted short recurrence columns"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,status,due,recur.indicator,mask,imask,parent.short")
        self.assertRegexpMatches(out, "1\sRecurring\s+\d{4}-\d{2}-\d{2}\s+R\s+-")
        self.assertRegexpMatches(out, "2\sPending\s+\d{4}-\d{2}-\d{2}\s+R\s+0\s+[0-9a-fA-F]{8}")

    def test_recurrence_formats_long(self):
        """Verify formatting of assorted long recurrence columns"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,status,due,recur.duration,mask,imask,parent.long")
        self.assertRegexpMatches(out, "1\sRecurring\s+\d{4}-\d{2}-\d{2}\s+P30D\s+-")
        self.assertRegexpMatches(out, "2\sPending\s+\d{4}-\d{2}-\d{2}\s+P30D\s+0\s+[0-9a-fA-F-]{36}")

    def test_recurrence_format_unrecognized(self):
        """Verify *.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,status,due,recur.donkey,mask,imask,parent.long")
        self.assertEqual(err, "Unrecognized column format 'recur.donkey'\n")

        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,status,due,recur.duration,mask,imask,parent.donkey")
        self.assertEqual(err, "Unrecognized column format 'parent.donkey'\n")


class TestProjectFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,project,description")
        cls.t.config("verbose",            "nothing")

        cls.t("add one   project:TOP")
        cls.t("add two   project:TOP.MIDDLE")
        cls.t("add three project:TOP.MIDDLE.BOTTOM")

    def setUp(self):
        """Executed before each test in the class"""

    def test_project_format_full(self):
        """Verify project.full formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,project.full,description")
        self.assertRegexpMatches(out, r'1\s+TOP\s+one')
        self.assertRegexpMatches(out, r'2\s+TOP.MIDDLE\s+two')
        self.assertRegexpMatches(out, r'3\s+TOP.MIDDLE.BOTTOM\s+three')

    def test_project_format_parent(self):
        """Verify project.parent formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,project.parent,description")
        self.assertRegexpMatches(out, r'1\s+TOP\s+one')
        self.assertRegexpMatches(out, r'2\s+TOP\s+two')
        self.assertRegexpMatches(out, r'3\s+TOP\s+three')

    def test_project_format_indented(self):
        """Verify project.indented formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,project.indented,description")
        self.assertRegexpMatches(out, r'1\s+TOP\s+one')
        self.assertRegexpMatches(out, r'2\s+MIDDLE\s+two')
        self.assertRegexpMatches(out, r'3\s+BOTTOM\s+three')

    def test_project_format_unrecognized(self):
        """Verify project.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,project.donkey,description")
        self.assertEqual(err, "Unrecognized column format 'project.donkey'\n")


class TestTagsFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,tags,description")
        cls.t.config("verbose",            "nothing")

        cls.t("add one +tag1 +tag2")

    def test_tags_format_list(self):
        """Verify tags.list formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,tags.list")
        self.assertRegexpMatches(out, r'1\s+tag1\stag2$')

    def test_tags_format_indicator(self):
        """Verify tags.indicator formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,tags.indicator")
        self.assertRegexpMatches(out, r'1\s+\+$')

    def test_tags_format_count(self):
        """Verify tags.count formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,tags.count")
        self.assertRegexpMatches(out, r'1\s+\[2\]$')

    def test_tags_format_unrecognized(self):
        """Verify tags.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,tags.donkey,description")
        self.assertEqual(err, "Unrecognized column format 'tags.donkey'\n")


class TestDateFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns", "id,due")
        cls.t.config("verbose",            "nothing")

        cls.t("add one due:yesterday")
        cls.t("add two due:tomorrow")

    def test_date_format_formatted(self):
        """Verify due.formatted formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,due.formatted")
        self.assertRegexpMatches(out, r'1\s+\d{4}-\d{2}-\d{2}')
        self.assertRegexpMatches(out, r'2\s+\d{4}-\d{2}-\d{2}')

    def test_date_format_julian(self):
        """Verify due.julian formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,due.julian")
        self.assertRegexpMatches(out, r'1\s+\d+\.\d+')
        self.assertRegexpMatches(out, r'2\s+\d+\.\d+')

    def test_date_format_epoch(self):
        """Verify due.epoch formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,due.epoch")
        self.assertRegexpMatches(out, r'1\s+\d{10}')
        self.assertRegexpMatches(out, r'2\s+\d{10}')

    def test_date_format_iso(self):
        """Verify due.iso formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,due.iso")
        self.assertRegexpMatches(out, r'1\s+\d{8}T\d{6}Z')
        self.assertRegexpMatches(out, r'2\s+\d{8}T\d{6}Z')

    def test_date_format_age(self):
        """Verify due.age formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,due.age")
        self.assertRegexpMatches(out, r'1\s+[0-9.]+d')
        self.assertRegexpMatches(out, r'2\s+-[0-9.]+[hmin]+')

    def test_date_format_remaining(self):
        """Verify due.remaining formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,due.remaining")
        self.assertRegexpMatches(out, r'1')
        self.assertRegexpMatches(out, r'2\s+\d+\S+')

    def test_date_format_relative(self):
        """Verify due.relative formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,due.relative")
        self.assertRegexpMatches(out, r'1\s+-[0-9.]+d')
        self.assertRegexpMatches(out, r'2\s+[0-9.]+[hmin]+')

    def test_date_format_countdown(self):
        """Verify due.countdown formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,due.countdown")
        self.assertRegexpMatches(out, r'1\s+\d+\S+')
        self.assertRegexpMatches(out, r'2\s+')

    def test_date_format_unrecognized(self):
        """Verify due.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,due.donkey,description")
        self.assertEqual(err, "Unrecognized column format 'due.donkey'\n")


class TestCustomColumns(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_unrecognized_column(self):
        """verify that using a bogus colum generates an error"""
        self.t.config("report.foo.description", "DESC")
        self.t.config("report.foo.columns",     "id,foo,description")
        self.t.config("report.foo.sort",        "id+")
        self.t.config("report.foo.filter",      "project:A")

        # Generate the usage screen, and locate the custom report on it.
        code, out, err = self.t.runError("foo")
        self.assertIn("Unrecognized column name 'foo'.", err)


class TestUDAFormats(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("report.xxx.columns",     "id,priority")
        cls.t.config("verbose",                "nothing")
        cls.t.config("uda.priority.indicator", "P")

        cls.t("add one priority:H")

    def test_uda_format_formatted(self):
        """Verify priority.default formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,priority.default")
        self.assertRegexpMatches(out, r'1\s+H')

    def test_uda_format_indicator(self):
        """Verify priority.indicator formatting"""
        code, out, err = self.t("xxx rc.report.xxx.columns:id,priority.indicator")
        self.assertRegexpMatches(out, r'1\s+P')

    def test_uda_format_unrecognized(self):
        """Verify priority.donkey formatting fails"""
        code, out, err = self.t.runError("xxx rc.report.xxx.columns:id,priority.donkey")
        self.assertEqual(err, "Unrecognized column format 'priority.donkey'\n")


        """
depends     list*             1 2 10
            count             [3]
            indicator         D

start       active*           ✓
        """

class TestFeature1061(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_columns(self):
        """1061: Verify 'task columns' works"""
        code, out, err = self.t("columns")
        self.assertIn("description", out)
        self.assertIn("uuid", out)
        self.assertIn("project", out)

    def test_columns_color(self):
        """1061: Verify 'task columns rc._forcecolor:1' works"""
        code, out, err = self.t("columns rc._forcecolor:1")
        self.assertIn("description", out)
        self.assertIn("uuid", out)
        self.assertIn("project", out)

    def test_columns_specific(self):
        """1061: Verify 'task columns escr' works"""
        code, out, err = self.t("columns escr")
        self.assertIn("description", out)
        self.assertNotIn("uuid", out)
        self.assertNotIn("project", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
