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

import datetime
import json
import sys
import numbers
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import UUID_REGEXP
from basetest.compat import STRING_TYPE

DATETIME_FORMAT = "%Y%m%dT%H%M%SZ"


class TestExportCommand(TestCase):
    def setUp(self):
        self.t = Task()
        self.t('add test')

    def export(self, id):
        code, out, err = self.t(("{0}".format(id), "rc.json.array=off", "export"))
        return json.loads(out.strip())

    def assertType(self, value, type):
        self.assertEqual(isinstance(value, type), True)

    def assertTimestamp(self, value):
        """
        Asserts that timestamp is exported as string in the correct format.
        """

        # Timestamps should be exported as strings
        self.assertType(value, STRING_TYPE)
        # And they should follow the %Y%m%dT%H%M%SZ format
        datetime.datetime.strptime(value, DATETIME_FORMAT)

    def assertString(self, value, expected_value=None, regexp=False):
        """
        Checks the type of value to be string, and that content is as passed in
        the expected_value argument. This can be either a string, or a compiled
        regular expression object.
        """

        self.assertType(value, STRING_TYPE)

        if expected_value is not None:
            if regexp:
                # Match to pattern if checking with regexp
                self.assertRegexpMatches(value, expected_value)
            else:
                # Equality match if checking with string
                self.assertEqual(value, expected_value)

    def assertNumeric(self, value, expected_value=None):
        """
        Checks the type of the value to be int, and that the expected value
        matches the actual value produced.
        """
        self.assertType(value, numbers.Real)
        if expected_value is not None:
            self.assertEqual(value, expected_value)

    def test_export_status(self):
        self.assertString(self.export(1)['status'], "pending")

    def test_export_uuid(self):
        self.assertString(self.export(1)['uuid'], UUID_REGEXP, regexp=True)

    def test_export_entry(self):
        self.assertTimestamp(self.export(1)['entry'])

    def test_export_description(self):
        self.assertString(self.export(1)['description'], "test")

    def test_export_start(self):
        self.t('1 start')
        self.assertTimestamp(self.export(1)['start'])

    def test_export_end(self):
        self.t('1 start')
        self.t.faketime("+5s")
        # After a task is "done" or "deleted", it does not have an ID by which
        # to filter it anymore. Add a tag to work around this.
        self.t('1 done +workaround')
        self.assertTimestamp(self.export('+workaround')['end'])

    def test_export_due(self):
        self.t('1 modify due:today')
        self.assertTimestamp(self.export(1)['due'])

    def test_export_wait(self):
        self.t('1 modify wait:tomorrow')
        self.assertTimestamp(self.export(1)['wait'])

    def test_export_modified(self):
        self.assertTimestamp(self.export(1)['modified'])

    def test_export_scheduled(self):
        self.t('1 modify schedule:tomorrow')
        self.assertTimestamp(self.export(1)['scheduled'])

    def test_export_recur(self):
        self.t('1 modify recur:daily due:today')
        self.assertString(self.export(1)['recur'], "daily")

    def test_export_project(self):
        self.t('1 modify project:Home')
        self.assertString(self.export(1)['project'], "Home")

    def test_export_priority(self):
        self.t('1 modify priority:H')
        self.assertString(self.export(1)['priority'], "H")

    def test_export_depends(self):
        self.t(('add', 'everything depends on me task'))
        self.t(('add', 'wrong, everything depends on me task'))
        self.t('1 modify depends:2,3')
        self.t.config('json.depends.array', 'on')

        deps = self.export(1)['depends']
        self.assertType(deps, list)
        self.assertEqual(len(deps), 2)

        for uuid in deps:
            self.assertString(uuid, UUID_REGEXP, regexp=True)

    def test_export_depends_oldformat(self):
        self.t(('add', 'everything depends on me task'))
        self.t(('add', 'wrong, everything depends on me task'))
        self.t('1 modify depends:2,3')

        code, out, err = self.t("rc.json.array=off rc.json.depends.array=off 1 export")
        deps = json.loads(out)["depends"]
        self.assertString(deps)
        self.assertEqual(len(deps.split(",")), 2)

        for uuid in deps.split(','):
            self.assertString(uuid, UUID_REGEXP, regexp=True)

    def test_export_urgency(self):
        self.t('add urgent task +urgent')

        # Urgency can be either integer or float
        self.assertNumeric(self.export(1)['urgency'])

    def test_export_numeric_uda(self):
        self.t.config('uda.estimate.type', 'numeric')
        self.t('add estimate:42 test numeric uda')
        self.assertNumeric(self.export('2')['estimate'], 42)

    def test_export_string_uda(self):
        self.t.config('uda.estimate.type', 'string')
        self.t('add estimate:big test string uda')
        self.assertString(self.export('2')['estimate'], 'big')

    def test_export_datetime_uda(self):
        self.t.config('uda.estimate.type', 'date')
        self.t('add estimate:eom test date uda')
        self.assertTimestamp(self.export('2')['estimate'])

    def test_export_duration_uda(self):
        self.t.config('uda.estimate.type', 'duration')
        self.t('add estimate:month test duration uda')
        self.assertString(self.export('2')['estimate'], 'P30D')


class TestExportCommandLimit(TestCase):
    def setUp(self):
        self.t = Task()

    def test_export_obeys_limit(self):
        """Verify that 'task export limit:1' is obeyed"""
        self.t('add one')
        self.t('add two')

        code, out, err = self.t("/o/ limit:1 export")
        self.assertIn("one", out)
        self.assertNotIn("two", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
