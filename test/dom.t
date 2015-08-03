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


class TestDOM(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()
        cls.t.config("dateformat", "YMD")
        cls.t("add one due:20110901")
        cls.t("add two due:1.due")
        cls.t("add three due:20110901 wait:due +tag1 +tag2")
        cls.t("3 annotate note")

    def test_dom_no_ref(self):
        """ DOM missing reference """
        code, out, err = self.t.runError("_get")
        self.assertEqual("No DOM reference specified.\n", err)

    def test_dom_bad_ref(self):
        """ DOM bad reference """
        code, out, err = self.t.runError("_get donkey")
        self.assertEqual("'donkey' is not a DOM reference.\n", err)

    def test_dom_task_ref(self):
        """ DOM reference to other task """
        code, out, err = self.t("_get 2.due")
        self.assertIn("2011-09-01T00:00:00", out)

    def test_dom_cli_ref(self):
        """ DOM reference to current command line """
        code, out, err = self.t("_get 3.wait")
        self.assertIn("2011-09-01T00:00:00", out)

    def test_dom_id_uuid_roundtrip(self):
        """ DOM id/uuid roundtrip """
        code, out, err = self.t("_get 1.uuid")
        uuid = out.strip()
        code, out, err = self.t("_get {0}.id".format(uuid))
        self.assertEqual("1\n", out)

    def test_dom_fail(self):
        """ DOM lookup of missing item """
        code, out, err = self.t("_get 4.description")
        self.assertEqual("\n", out)

    def test_dom_tags(self):
        """ DOM 3.tags """
        code, out, err = self.t("_get 3.tags")
        self.assertEqual("tag1,tag2\n", out)

    def test_dom_tags_tag1(self):
        """ DOM 3.tags.tag1 """
        code, out, err = self.t("_get 3.tags.tag1")
        self.assertEqual("tag1\n", out)

    def test_dom_tags_OVERDUE(self):
        """ DOM 3.tags.OVERDUE """
        code, out, err = self.t("_get 3.tags.OVERDUE")
        self.assertEqual("OVERDUE\n", out)

    def test_dom_due_year(self):
        """ DOM 3.due.year """
        code, out, err = self.t("_get 3.due.year")
        self.assertEqual("2011\n", out)

    def test_dom_due_month(self):
        """ DOM 3.due.month """
        code, out, err = self.t("_get 3.due.month")
        self.assertEqual("9\n", out)

    def test_dom_due_day(self):
        """ DOM 3.due.day """
        code, out, err = self.t("_get 3.due.day")
        self.assertEqual("1\n", out)

    def test_dom_due_week(self):
        """ DOM 3.due.week """
        code, out, err = self.t("_get 3.due.week")
        self.assertEqual("36\n", out)

    def test_dom_due_weekday(self):
        """ DOM 3.due.weekday """
        code, out, err = self.t("_get 3.due.weekday")
        self.assertEqual("4\n", out)

    def test_dom_due_hour(self):
        """ DOM 3.due.hour """
        code, out, err = self.t("_get 3.due.hour")
        self.assertEqual("0\n", out)

    def test_dom_due_minute(self):
        """ DOM 3.due.minute """
        code, out, err = self.t("_get 3.due.minute")
        self.assertEqual("0\n", out)

    def test_dom_due_second(self):
        """ DOM 3.due.second """
        code, out, err = self.t("_get 3.due.second")
        self.assertEqual("0\n", out)

    def test_dom_annotation_entry(self):
        """ DOM 3.annotations.1.entry """
        code, out, err = self.t("_get 3.annotations.1.entry")
        self.assertRegexpMatches(out, r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}")

    def test_dom_annotation_entry_second(self):
        """ DOM 3.annotations.1.entry """
        code, out, err = self.t("_get 3.annotations.1.entry.second")
        self.assertRegexpMatches(out, r"\d{1,2}")

    def test_dom_annotation_description(self):
        """ DOM 3.annotations.1.description """
        code, out, err = self.t("_get 3.annotations.1.description")
        self.assertIn("note\n", out)

    def test_dom_system_version(self):
        """ DOM system.version """
        code, out, err = self.t("_get system.version")
        self.assertEqual(code, 0)
        self.assertRegexpMatches(out, r"\d\.\d+\.\d+")

    def test_dom_system_os(self):
        """ DOM system.os """
        code, out, err = self.t("_get system.os")
        self.assertEqual(code, 0)
        self.assertEqual(len(out) > 4, True)
        self.assertNotIn("<unknown>", out)

    def test_dom_context_program(self):
        """ DOM context.program """
        code, out, err = self.t("_get context.program")
        self.assertEqual(code, 0)
        self.assertIn("task", out)

    def test_dom_context_args(self):
        """ DOM context.args """
        code, out, err = self.t("_get context.args")
        self.assertEqual(code, 0)
        self.assertIn("task _get context.args", out)

    def test_dom_context_width(self):
        """ DOM context.width """
        code, out, err = self.t("_get context.width")
        self.assertEqual(code, 0)
        self.assertRegexpMatches(out, r"\d+")

    def test_dom_context_height(self):
        """ DOM context.height """
        code, out, err = self.t("_get context.height")
        self.assertEqual(code, 0)
        self.assertRegexpMatches(out, r"\d+")

    def test_dom_rc_name(self):
        """ DOM rc.dateformat """
        code, out, err = self.t("_get rc.dateformat")
        self.assertEqual(code, 0)
        self.assertIn("YMD", out)

    def test_dom_rc_missing(self):
        """ DOM rc.missing """
        code, out, err = self.t("_get rc.missing")
        self.assertEqual("\n", out)

    def test_dom_attribute_missing(self):
        """DOM 1.end (missing)"""
        code, out, err = self.t("_get 1.end")
        self.assertEqual("\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
