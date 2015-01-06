#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

import datetime
import json
import sys
import time
import os
import re
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase

# TODO: Move to common file with all regexpes
UUID_REGEXP = r'[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}'
DATETIME_FORMAT = "%Y%m%dT%H%M%SZ"


class TestExportCommand(TestCase):
    def setUp(self):
        self.t = Task()
        self.t(('add', 'test'))

    def export(self, identifier):
        return json.loads(self.t((str(identifier), 'export'))[1].strip())

    def assertType(self, value, type):
        self.assertEqual(isinstance(value, type), True)

    def assertTimestamp(self, value):
        """
        Asserts that timestamp is exported as string in the correct format.
        """

        # Timestamps should be exported as strings
        self.assertType(value, unicode)
        # And they should follow the %Y%m%dT%H%M%SZ format
        datetime.datetime.strptime(value, DATETIME_FORMAT)

    def assertString(self, value, expected_value=None, regexp=False):
        """
        Checks the type of value to be string, and that context is as passed in
        the expected_value argument. This can be either a string, or a compiled
        regular expression object.
        """

        self.assertType(value, unicode)

        if expected_value is not None:
            if regexp:
                # Match to pattern if checking with regexp
                self.assertRegexpMatches(value, expected_value)
            else:
                # Equality match if checking with string
                self.assertEqual(value, expected_value)

    def assertInt(self, value, expected_value=None):
        """
        Checks the type of the value to be int, and that the expected value
        matches the actual value produced.
        """
        self.assertType(value, int)
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
        self.t(('1', 'start'))
        self.assertTimestamp(self.export(1)['start'])

    def test_export_end(self):
        self.t(('1', 'start'))
        time.sleep(2)
        self.t(('1', 'done'))
        self.assertTimestamp(self.export(1)['end'])

    def test_export_due(self):
        self.t(('1', 'modify', 'due:today'))
        self.assertTimestamp(self.export(1)['due'])

    def test_export_wait(self):
        self.t(('1', 'modify', 'wait:tomorrow'))
        self.assertTimestamp(self.export(1)['wait'])

    def test_export_modified(self):
        self.assertTimestamp(self.export(1)['modified'])

    def test_export_scheduled(self):
        self.t(('1', 'modify', 'schedule:tomorrow'))
        self.assertTimestamp(self.export(1)['scheduled'])

    def test_export_recur(self):
        self.t(('1', 'modify', 'recur:daily', 'due:today'))
        self.assertString(self.export(1)['recur'], "daily")

    def test_export_project(self):
        self.t(('1', 'modify', 'project:Home'))
        self.assertString(self.export(1)['project'], "Home")

    def test_export_priority(self):
        self.t(('1', 'modify', 'priority:H'))
        self.assertString(self.export(1)['priority'], "H")

    def test_export_depends(self):
        self.t(('add', 'everything depends on me task'))
        self.t(('add', 'wrong, everything depends on me task'))
        self.t(('1', 'modify', 'depends:2,3'))

        values = self.export(1)['depends']
        self.assertString(values)

        for uuid in values.split(','):
            self.assertString(uuid, UUID_REGEXP, regexp=True)

    def test_export_numeric_uda(self):
        self.t.config('uda.estimate.type', 'numeric')
        self.t(('add', 'estimate:42', 'test numeric uda'))
        self.assertInt(self.export('2')['estimate'], int)

    def test_export_string_uda(self):
        self.t.config('uda.estimate.type', 'string')
        self.t(('add', 'estimate:big', 'test string uda'))
        self.assertString(self.export('2')['estimate'], 'big')

    def test_export_datetime_uda(self):
        self.t.config('uda.estimate.type', 'date')
        self.t(('add', 'estimate:eom', 'test date uda'))
        self.assertTimestamp(self.export('2')['estimate'])

    def test_export_duration_uda(self):
        self.t.config('uda.estimate.type', 'duration')
        self.t(('add', 'estimate:month', 'test duration uda'))
        self.assertString(self.export('2')['estimate'], 'month')

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
