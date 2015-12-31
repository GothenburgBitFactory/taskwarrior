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
import time
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.meta import MetaTest


class MetaTestSorting(MetaTest):
    """Helper metaclass to simplify test logic below (TestSorting)

    Creates test_methods in the TestCase class dynamically named after the
    filter used.
    """
    @staticmethod
    def make_function(classname, *args, **kwargs):
        _filter, expectations = args

        def test(self):
            # ### Body of the usual test_testcase ### #
            code, out, err = self.t(
                "rc.report.{0}.sort:{1} {0}".format(self._report, _filter)
            )

            for expected in expectations:
                regex = re.compile(expected, re.DOTALL)
                self.assertRegexpMatches(out, regex)

        # Title of test in report
        test.__doc__ = "{0} sort:{1}".format(classname, _filter)

        return test


class TestSorting(TestCase):
    __metaclass__ = MetaTestSorting

    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        # Report to use when running this class's tests
        cls._report = "list"

        cls.t("add                                    zero")
        cls.t("add priority:H project:A due:yesterday one")
        cls.t("add priority:M project:B due:today     two")
        cls.t("add priority:L project:C due:tomorrow  three")
        cls.t("add priority:H project:C due:today     four")
        cls.t("2 start")

    TESTS = (
        # Filter                           # Expected matches/outputs

        # Single sort column.
        ('priority-',                       ('(?:one.+four|four.+one).+two.+three.+zero',)),
        ('priority+',                       ('zero.+three.+two.+(?:one.+four|four.+one)',)),
        ('project-',                        ('(?:three.+four|four.+three).+two.+one.+zero',)),
        ('project+',                        ('zero.+one.+two.+(?:three.+four|four.+three)',)),
        ('start-',                          ('one.+zero', 'one.+two', 'one.+three', 'one.+four',)),
        ('start+',                          ('one.+zero', 'one.+two', 'one.+three', 'one.+four',)),
        ('due-',                            ('three.+(?:two.+four|four.+two).+one.+zero',)),
        ('due+',                            ('one.+(?:two.+four|four.+two).+three.+zero',)),
        ('description-',                    ('zero.+two.+three.+one.+four',)),
        ('description+',                    ('four.+one.+three.+two.+zero',)),

        # Two sort columns.
        ('priority-,project-',              ('four.+one.+two.+three.+zero',)),
        ('priority-,project+',              ('one.+four.+two.+three.+zero',)),
        ('priority+,project-',              ('zero.+three.+two.+four.+one',)),
        ('priority+,project+',              ('zero.+three.+two.+one.+four',)),

        ('priority-,start-',                ('one.+four.+two.+three.+zero',)),
        ('priority-,start+',                ('one.+four.+two.+three.+zero',)),
        ('priority+,start-',                ('zero.+three.+two.+one.+four',)),
        ('priority+,start+',                ('zero.+three.+two.+one.+four',)),

        ('priority-,due-',                  ('four.+one.+two.+three.+zero',)),
        ('priority-,due+',                  ('one.+four.+two.+three.+zero',)),
        ('priority+,due-',                  ('zero.+three.+two.+four.+one',)),
        ('priority+,due+',                  ('zero.+three.+two.+one.+four',)),

        ('priority-,description-',          ('one.+four.+two.+three.+zero',)),
        ('priority-,description+',          ('four.+one.+two.+three.+zero',)),
        ('priority+,description-',          ('zero.+three.+two.+one.+four',)),
        ('priority+,description+',          ('zero.+three.+two.+four.+one',)),

        ('project-,priority-',              ('four.+three.+two.+one.+zero',)),
        ('project-,priority+',              ('three.+four.+two.+one.+zero',)),
        ('project+,priority-',              ('zero.+one.+two.+four.+three',)),
        ('project+,priority+',              ('zero.+one.+two.+three.+four',)),

        ('project-,start-',                 ('three.+four.+two.+one.+zero',)),
        ('project-,start+',                 ('(?:four.+three|three.+four).+two.+one.+zero',)),
        ('project+,start-',                 ('zero.+one.+two.+three.+four',)),
        ('project+,start+',                 ('zero.+one.+two.+(?:four.+three|three.+four)',)),

        ('project-,due-',                   ('three.+four.+two.+one.+zero',)),
        ('project-,due+',                   ('four.+three.+two.+one.+zero',)),
        ('project+,due-',                   ('zero.+one.+two.+three.+four',)),
        ('project+,due+',                   ('zero.+one.+two.+four.+three',)),

        ('project-,description-',           ('three.+four.+two.+one.+zero',)),
        ('project-,description+',           ('four.+three.+two.+one.+zero',)),
        ('project+,description-',           ('zero.+one.+two.+three.+four',)),
        ('project+,description+',           ('zero.+one.+two.+four.+three',)),

        ('start-,priority-',                ('one.+four.+two.+three.+zero',)),
        ('start-,priority+',                ('one.+zero.+three.+two.+four',)),
        ('start+,priority-',                ('one.+four.+two.+three.+zero',)),
        ('start+,priority+',                ('one.+zero.+three.+two.+four',)),

        ('start-,project-',                 ('one.+(?:three.+four|four.+three).+two.+zero',)),
        ('start-,project+',                 ('one.+zero.+two.+(?:three.+four|four.+three)',)),
        ('start+,project-',                 ('one.+(?:three.+four|four.+three).+two.+zero',)),
        ('start+,project+',                 ('one.+zero.+two.+(?:three.+four|four.+three)',)),

        ('start-,due-',                     ('one.+three.+(?:four.+two|two.+four).+zero',)),
        ('start-,due+',                     ('one.+(?:four.+two|two.+four).+three.+zero',)),
        ('start+,due-',                     ('one.+three.+(?:four.+two|two.+four).+zero',)),
        ('start+,due+',                     ('one.+(?:four.+two|two.+four).+three.+zero',)),

        ('start-,description-',             ('one.+zero.+two.+three.+four',)),
        ('start-,description+',             ('one.+four.+three.+two.+zero',)),
        ('start+,description-',             ('one.+zero.+two.+three.+four',)),
        ('start+,description+',             ('one.+four.+three.+two.+zero',)),

        ('due-,priority-',                  ('three.+four.+two.+one.+zero',)),
        ('due-,priority+',                  ('three.+two.+four.+one.+zero',)),
        ('due+,priority-',                  ('one.+four.+two.+three.+zero',)),
        ('due+,priority+',                  ('one.+two.+four.+three.+zero',)),

        ('due-,project-',                   ('three.+four.+two.+one.+zero',)),
        ('due-,project+',                   ('three.+two.+four.+one.+zero',)),
        ('due+,project-',                   ('one.+four.+two.+three.+zero',)),
        ('due+,project+',                   ('one.+two.+four.+three.+zero',)),

        ('due-,start-',                     ('three.+(?:four.+two|two.+four).+one.+zero',)),
        ('due-,start+',                     ('three.+(?:four.+two|two.+four).+one.+zero',)),
        ('due+,start-',                     ('one.+(?:four.+two|two.+four).+three.+zero',)),
        ('due+,start+',                     ('one.+(?:four.+two|two.+four).+three.+zero',)),

        ('due-,description-',               ('three.+two.+four.+one.+zero',)),
        ('due-,description+',               ('three.+four.+two.+one.+zero',)),
        ('due+,description-',               ('one.+two.+four.+three.+zero',)),
        ('due+,description+',               ('one.+four.+two.+three.+zero',)),

        ('description-,priority-',          ('zero.+two.+three.+one.+four',)),
        ('description-,priority+',          ('zero.+two.+three.+one.+four',)),
        ('description+,priority-',          ('four.+one.+three.+two.+zero',)),
        ('description+,priority+',          ('four.+one.+three.+two.+zero',)),

        ('description-,project-',           ('zero.+two.+three.+one.+four',)),
        ('description-,project+',           ('zero.+two.+three.+one.+four',)),
        ('description+,project-',           ('four.+one.+three.+two.+zero',)),
        ('description+,project+',           ('four.+one.+three.+two.+zero',)),

        ('description-,start-',             ('zero.+two.+three.+one.+four',)),
        ('description-,start+',             ('zero.+two.+three.+one.+four',)),
        ('description+,start-',             ('four.+one.+three.+two.+zero',)),
        ('description+,start+',             ('four.+one.+three.+two.+zero',)),

        ('description-,due-',               ('zero.+two.+three.+one.+four',)),
        ('description-,due+',               ('zero.+two.+three.+one.+four',)),
        ('description+,due-',               ('four.+one.+three.+two.+zero',)),
        ('description+,due+',               ('four.+one.+three.+two.+zero',)),

        # Four sort columns.
        ('start+,project+,due+,priority+',  ('one.+zero.+two.+four.+three',)),
        ('project+,due+,priority+,start+',  ('zero.+one.+two.+four.+three',)),
    )


class TestBug438(TestCase):
    __metaclass__ = MetaTestSorting

    # Bug #438: Reports sorting by end, start, and entry are ordered
    #           incorrectly, if time is included.
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        # Report to use when running this class's tests
        cls._report = "foo"

        cls.t.config("dateformat", "SNHDMY")
        cls.t.config("report.foo.columns", "entry,start,end,description")
        cls.t.config("report.foo.dateformat", "SNHDMY")

        # Preparing data:
        #  2 tasks created in the past, 1 second apart
        #  2 tasks created in the past, and started 10 seconds later
        #  2 tasks created in the past, and finished 20 seconds later

        stamp = int(time.time())
        cls.t("add one older entry:{0}".format(stamp))
        stamp += 1
        cls.t("add one newer entry:{0}".format(stamp))

        start = stamp + 10
        cls.t("add two older entry:{0} start:{1}".format(stamp, start))
        start += 1
        cls.t("add two newer entry:{0} start:{1}".format(stamp, start))

        end = start + 10
        cls.t("log three older entry:{0} end:{1}".format(stamp, end))
        end += 1
        cls.t("log three newer entry:{0} end:{1}".format(stamp, end))

    TESTS = {
        ("entry+", ("one older.+one newer",)),
        ("entry-", ("one newer.+one older",)),
        ("start+", ("two older.+two newer",)),
        ("start-", ("two newer.+two older",)),
        ("end+",   ("three older.+three newer",)),
        ("end-",   ("three newer.+three older",)),
    }


class TestSortNone(TestCase):
    def setUp(self):
        self.t = Task()

    def test_sort_none(self):
        """Verify that 'sort:none' removes all sorting"""
        self.t("add one")
        self.t("add two")
        self.t("add three")
        code, out, err = self.t("_get 1.uuid 2.uuid 3.uuid")
        uuid1, uuid2, uuid3 = out.strip().split(' ')
        code, out, err = self.t("%s %s %s list rc.report.list.sort:none rc.report.list.columns:id,description rc.report.list.labels:id,desc" % (uuid2, uuid3, uuid1))
        self.assertRegexpMatches(out, ' 2 two\n 3 three\n 1 one')


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
