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

# NOTE Test classes are ordered alphabetically


class TestBaseUda(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("uda.extra.label", "Extra")
        self.t.config("report.uda.description", "UDA Test")
        self.t.config("report.uda.columns", "id,extra,description")
        self.t.config("report.uda.sort", "extra,description")
        self.t.config("report.uda.labels", "ID,Extra,Description")


class TestUdaCommand(TestBaseUda):
    def setUp(self):
        super(TestUdaCommand, self).setUp()
        self.t.config("uda.extra.type", "string")

    def test_uda_command(self):
        """The 'udas' command should list 'priority' and 'extra'"""
        code, out, err = self.t("udas")
        self.assertIn("priority", out)
        self.assertIn("extra", out)

    def test_uda_helper_command(self):
        """The '_udas' helper command should list 'priority' and 'extra'"""
        code, out, err = self.t("_udas")
        self.assertIn("priority", out)
        self.assertIn("extra", out)


class TestUdaCommandOrphans(TestCase):
    def setUp(self):
        self.t = Task()

    def test_uda_command_orphans(self):
        """The 'udas' command should list 'orphans'"""
        # Create a task with a UDA.
        self.t.config("uda.orphan.label", "orphan")
        self.t.config("uda.orphan.type", "string")
        self.t("add one orphan:Annie")

        # Convert the UDA to an orphan.
        self.t.del_config("uda.orphan.label")
        self.t.del_config("uda.orphan.type")

        code, out, err = self.t("udas")
        self.assertIn("1 UDA defined", out)
        self.assertIn("1 Orphan UDA", out)


class TestUdaDate(TestBaseUda):
    def setUp(self):
        super(TestUdaDate, self).setUp()

        self.t.config("uda.extra.type", "date")
        self.t.config("dateformat", "m/d/Y")

    def test_uda_date_task(self):
        """Add tasks with and without a UDA date"""

        code, out, err = self.t("add with extra:tomorrow")
        self.assertIn("Created task", out)

        code, out, err = self.t("add without")
        self.assertIn("Created task", out)

        code, out, err = self.t("uda")
        self.assertRegexpMatches(out, "1\s+[\d\/]+\s+with")
        self.assertRegexpMatches(out, "2\s+without")

    def test_uda_bad_date_task(self):
        """Add tasks with an invalid UDA date"""
        code, out, err = self.t.runError("add bad extra:bad_date")
        self.assertNotIn("Created task", out)
        self.assertIn("not a valid date", err)


class TestUdaDefault(TestBaseUda):
    def setUp(self):
        super(TestUdaDefault, self).setUp()

        self.t.config("uda.extra.type", "numeric")
        self.t.config("uda.smell.type", "string")
        self.t.config("uda.smell.label", "Smell")
        self.t.config("uda.smell.values", "weak,strong")
        self.t.config("uda.smell.default", "weak")

        self.t.config("report.uda.columns", "id,smell,extra,description")
        self.t.config("report.uda.sort", "id")
        self.t.config("report.uda.labels", "ID,Smell,Size,Description")

    def test_uda_nondefault_task(self):
        """Add tasks with non default UDA"""

        code, out, err = self.t("add one smell:strong")
        self.assertIn("Created task", out)

        code, out, err = self.t("uda")
        self.assertRegexpMatches(out, "1\s+strong\s+one")

    def test_uda_default_task(self):
        """Add tasks with default UDA"""

        code, out, err = self.t("add two")
        self.assertIn("Created task", out)

        code, out, err = self.t("uda")
        self.assertRegexpMatches(out, "1\s+weak\s+two")

    def test_uda_without_default_task(self):
        """Add tasks without default UDA"""

        code, out, err = self.t("add three extra:10")
        self.assertIn("Created task", out)

        code, out, err = self.t("uda")
        self.assertRegexpMatches(out, "1\s+weak\s+10\s+three")


class TestUdaDuration(TestBaseUda):
    def setUp(self):
        super(TestUdaDuration, self).setUp()

        self.t.config("uda.extra.type", "duration")

    def test_uda_duration_task(self):
        """Add tasks with and without a UDA duration"""
        code, out, err = self.t("add with extra:1day")
        code, out, err = self.t("add without")

        code, out, err = self.t("_get 1.extra")
        self.assertEqual("P1D\n", out)

        code, out, err = self.t("_get 2.extra")
        self.assertEqual("\n", out)

        # Ensure 'extra' is stored in original form.
        code, out, err = self.t("1 export")
        self.assertRaisesRegexp(out, '"extra":"P1D"')

    def test_uda_bad_duration_task(self):
        """Add tasks with an invalid UDA duration"""
        code, out, err = self.t.runError("add bad extra:bad_duration")
        self.assertNotIn("Created task", out)
        self.assertIn("The duration value 'bad_duration' is not supported",
                      err)


class TestUdaNumeric(TestBaseUda):
    def setUp(self):
        super(TestUdaNumeric, self).setUp()

        self.t.config("uda.extra.type", "numeric")

    def test_uda_numeric_task(self):
        """Add tasks with and without a UDA numeric"""

        code, out, err = self.t("add with extra:123")
        self.assertIn("Created task", out)

        code, out, err = self.t("add without")
        self.assertIn("Created task", out)

        code, out, err = self.t("uda")
        self.assertRegexpMatches(out, "1\s+\d+\s+with")
        self.assertRegexpMatches(out, "2\s+without")

    def test_uda_bad_numeric_task(self):
        """Add tasks with an invalid UDA numeric"""
        code, out, err = self.t.runError("add bad extra:bad_numeric")
        self.assertNotIn("Created task", out)
        self.assertIn("The value 'bad_numeric' is not a valid numeric value", err)


class TestUdaString(TestBaseUda):
    def setUp(self):
        super(TestUdaString, self).setUp()

        self.t.config("uda.extra.type", "string")

    def test_uda_string_task(self):
        """Add tasks with and without a UDA string"""

        code, out, err = self.t("add with extra:'one two'")
        self.assertIn("Created task", out)
        code, out, err = self.t("add without")
        self.assertIn("Created task", out)

        code, out, err = self.t("uda")
        self.assertRegexpMatches(out, "1\s+one two\s+with")
        self.assertRegexpMatches(out, "2\s+without")


class TestUdaValue(TestBaseUda):
    def setUp(self):
        super(TestUdaValue, self).setUp()

        self.t.config("uda.extra.type", "string")
        self.t.config("uda.extra.values", "weak,strong")

    def test_uda_value_task(self):
        """Add tasks with valid UDA values"""

        code, out, err = self.t("add one extra:weak")
        self.assertIn("Created task", out)
        code, out, err = self.t("add two extra:strong")
        self.assertIn("Created task", out)

        code, out, err = self.t("uda")
        self.assertRegexpMatches(out, "1\s+weak\s+one")
        self.assertRegexpMatches(out, "2\s+strong\s+two")

    def test_uda_invalid_value_task(self):
        """Add tasks with invalid UDA value"""

        code, out, err = self.t("add one extra:strong")
        self.assertIn("Created task", out)

        code, out, err = self.t.runError("add two extra:toxic")
        self.assertIn("The 'extra' attribute does not allow a value of "
                      "'toxic'", err)

        code, out, err = self.t("uda")
        self.assertRegexpMatches(out, "1\s+strong\s+one")
        self.assertNotRegexpMatches(out, "1\s+toxic\s+two")


class TestBug1063(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("uda.foo.type", "numeric")
        self.t.config("uda.foo.label", "Foo")
        self.t.config("report.bar.columns", "foo,description")
        self.t.config("report.bar.description", "Bar")
        self.t.config("report.bar.labels", "Foo,Desc")
        self.t.config("report.bar.sort", "foo-")

    def test_sortable_uda(self):
        """1063: numeric UDA fields are sortable

        Reported as bug 1063
        """

        self.t("add four foo:4")
        self.t("add one foo:1")
        self.t("add three foo:3")
        self.t("add two foo:2")

        code, out, err = self.t("bar")
        expected = re.compile("4.+3.+2.+1", re.DOTALL)  # dot matches \n too
        self.assertRegexpMatches(out, expected)

        code, out, err = self.t("bar rc.report.bar.sort=foo+")
        expected = re.compile("1.+2.+3.+4", re.DOTALL)  # dot matches \n too
        self.assertRegexpMatches(out, expected)


class TestBug21(TestCase):
    def setUp(self):
        self.t = Task()

    def test_almost_UDA(self):
        """21: do not match a UDA if not followed by colon"""
        self.t.config('uda.foo.type', 'string')
        self.t.config('uda.foo.label', 'FOO')

        self.t("add this is a foo bar")
        code, out, err = self.t("1 info")

        self.assertIn("this is a foo bar", out)


class Test1447(TestCase):
    def setUp(self):
        self.t = Task()

    def test_filter_uda(self):
        """1447: Verify ability to filter on empty UDA"""
        self.t.config('uda.sep.type', 'string')
        self.t('add one')
        self.t('add two sep:foo')
        code, out, err = self.t('sep: list')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('one', out)


class Test1542(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("uda.bugid.type", "numeric")
        self.t.config("uda.bugid.label", "BugID")

    def test_large_numeric_uda_retains_value(self):
        """
        1542: Make sure the numeric UDA value 1187962 does not get converted to
        scientific notation on export.
        """
        self.t('add large bugid:1187962')
        code, out, err = self.t('1 export')
        self.assertIn("\"bugid\":1187962,", out)

    def test_small_numeric_uda_retains_value(self):
        """
        1542: Make sure the numeric UDA value 43.21 does not get converted to
        integer on export.
        """
        self.t('add small bugid:43.21')
        code, out, err = self.t('1 export')
        self.assertIn("\"bugid\":43.21", out)


class TestBug1622(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_uda_duration_expression(self):
        """1622: Verify that a UDA of type 'duration' accepts an expression"""
        self.t.config("uda.dur.type", "duration")
        self.t("add Run 3 miles dur:33min+18s")

        code, out, err = self.t("_get 1.dur")
        self.assertEqual("PT33M18S\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
