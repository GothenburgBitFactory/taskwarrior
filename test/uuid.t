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


class TestUUID(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("dateformat", "m/d/Y")

        self.t("import -", input="""[
        {"description":"one", "entry":"1315260230", "status":"pending", "uuid":"9deed7ca-843d-4259-b2c4-40ce73e8e4f3"},
        {"description":"two", "entry":"1315260230", "status":"pending", "uuid":"0f4c83d2-552f-4108-ae3f-ccc7959f84a3"},
        {"description":"three", "entry":"1315260230", "status":"pending", "uuid":"aa4abef1-1dc5-4a43-b6a0-7872df3094bb"},
        {"description":"four", "end":"1315260230", "entry":"1315260230", "status":"completed", "uuid":"ea3b4822-574c-464b-8025-7f7be9f3cc57"},
        {"description":"ssttaarrtt", "entry":"1315335826", "start":"1315338535", "status":"pending", "uuid":"d71d3566-7a6b-4c32-8f0b-6de75bb9397b"},
        {"description":"five", "end":"1315260230", "entry":"1315260230", "status":"completed", "uuid":"0f38b97e-3081-4e75-a1be-65ed3712ea4d"},
        {"description":"six", "end":"1315338826", "entry":"1315338726", "status":"completed", "uuid":"12345678-1234-1234-1234-123456789012"},
        {"description":"seven", "end":"1315338826", "entry":"1315338726", "status":"completed", "uuid":"abcdefab-abcd-abcd-abcd-abcdefabcdef"},
        {"description":"eenndd", "end":"1315335841", "entry":"1315335841", "start":"1315338516", "status":"completed", "uuid":"727baa6c-65b8-485e-a810-e133e3cd83dc"},
        {"description":"UUNNDDOO", "end":"1315338626", "entry":"1315338626", "status":"completed", "uuid":"c1361003-948e-43e8-85c8-15d28dc3c71c"}
        ]""")

    def _config_unittest_report(self):
        self.t.config("report.unittest.columns", "id,entry,start,description")
        self.t.config("report.unittest.filter", "status:pending")
        self.t.config("report.unittest.sort", "id")

    def test_uuid_modify_pending(self):
        """Modify with UUID + report pending"""
        self.t("9deed7ca-843d-4259-b2c4-40ce73e8e4f3 modify ONE")
        self.t("2 modify TWO")
        code, out, err = self.t("list")

        self.assertIn("ONE", out)
        self.assertIn("TWO", out)
        self.assertIn("three", out)
        self.assertIn("ssttaarrtt", out)
        self.assertNotIn("four", out)
        self.assertNotIn("five", out)
        self.assertNotIn("six", out)
        self.assertNotIn("seven", out)
        self.assertNotIn("eenndd", out)
        self.assertNotIn("UUNNDDOO", out)

    def test_uuid_modify_completed(self):
        """Modify with UUID + report completed"""
        self.t("ea3b4822-574c-464b-8025-7f7be9f3cc57 modify FOUR")
        code, out, err = self.t("completed")

        self.assertNotIn("ONE", out)
        self.assertNotIn("TWO", out)
        self.assertNotIn("three", out)
        self.assertNotIn("ssttaarrtt", out)
        self.assertIn("FOUR", out)
        self.assertIn("five", out)
        self.assertIn("six", out)
        self.assertIn("seven", out)
        self.assertIn("eenndd", out)
        self.assertIn("UUNNDDOO", out)

    def test_uuid_modify_status(self):
        """Modify status: with UUID"""
        self.t("c1361003-948e-43e8-85c8-15d28dc3c71c modify status:pending")

        code, out, err = self.t("list")
        self.assertIn("UUNNDDOO", out)

        code, out, err = self.t("completed")
        self.assertNotIn("UUNNDDOO", out)

    def test_uuid_modify_start(self):
        """Modify start: with UUID"""
        self._config_unittest_report()

        self.t("d71d3566-7a6b-4c32-8f0b-6de75bb9397b modify start:12/31/2010")

        code, out, err = self.t("unittest")
        self.assertIn("12/31/2010", out)

    def test_uuid_modify_end(self):
        """Modify end: with UUID"""
        self.t("727baa6c-65b8-485e-a810-e133e3cd83dc modify end:12/31/2010")

        code, out, err = self.t("completed")
        self.assertIn("12/31/2010", out)

    def test_uuid_modify_start_end(self):
        """Modify start: and end: with UUID"""
        self._config_unittest_report()

        self.t("aa4abef1-1dc5-4a43-b6a0-7872df3094bb modify entry:12/30/2010")
        self.t("aa4abef1-1dc5-4a43-b6a0-7872df3094bb modify start:1/1/2011")

        code, out, err = self.t("unittest")
        self.assertIn("12/30/2010", out)
        self.assertIn("1/1/2011", out)

    def test_numerical_uuid(self):
        """Using numerical UUID"""
        # NOTE: Reported on TW-1636
        self.t("12345678-1234-1234-1234-123456789012 modify status:completed")

        code, out, err = self.t("_get 12345678-1234-1234-1234-123456789012.status")
        self.assertIn("completed\n", out)

        code, out, err = self.t("12345678-1234-1234-1234-123456789012 export")
        self.assertIn('"description":"six"', out)

    def test_numerical_short_uuid(self):
        """Using numerical UUID in the short form"""
        # NOTE: Reported on TW-1636
        self.t("12345678 modify status:pending")

        code, out, err = self.t("_get 12345678.status")
        self.assertIn("pending\n", out)

        code, out, err = self.t("12345678 export")
        self.assertIn('"description":"six"', out)

    def test_alpha_uuid(self):
        """Using alphabetic UUID"""
        # NOTE: complement numerical only reported on TW-1636
        self.t("abcdefab-abcd-abcd-abcd-abcdefabcdef modify status:completed")

        code, out, err = self.t("_get abcdefab-abcd-abcd-abcd-abcdefabcdef.status")
        self.assertIn("completed\n", out)

        code, out, err = self.t("abcdefab-abcd-abcd-abcd-abcdefabcdef export")
        self.assertIn('"description":"seven"', out)

    def test_alpha_short_uuid(self):
        """Using alphabetic UUID in the short form"""
        # NOTE: complement numerical only reported on TW-1636
        self.t("abcdefab modify status:pending")

        code, out, err = self.t("_get abcdefab.status")
        self.assertIn("pending\n", out)

        code, out, err = self.t("abcdefab export")
        self.assertIn('"description":"seven"', out)


class TestUUIDuplicates(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_uuid_duplicates(self):
        """Verify that duplicating tasks, and recurring tasks do no create duplicates UUIDs"""
        self.t("add simple")
        self.t("1 duplicate")
        self.t("add periodic recur:daily due:yesterday")
        self.t("list") # GC/handleRecurrence

        uuids = list()
        for id in range(1,7):
            code, out, err = self.t("_get %d.uuid" % id)
            uuids.append(out.strip())

        self.assertEqual(len(uuids), len(set(uuids)))

        code, out, err = self.t("diag")
        self.assertIn("No duplicates found", out)


class TestBug954(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add foo")

    def test_deletion_by_uuid(self):
        """954: Verify deletion using extant UUID"""
        code, out, err = self.t("_get 1.uuid")
        code, out, err = self.t(out.strip() + " delete", input="y\n")
        self.assertIn("Deleting task 1 'foo'", out)

    def test_deletion_by_missing_uuid(self):
        """954: Verify deletion using missing UUID"""
        code, out, err = self.t.runError("874e146d-07a2-2d2c-7808-a76e74b1a332 delete")
        self.assertIn("No tasks specified.", err)


class TestFeature891(TestCase):
    @classmethod
    def setUp(self):
        self.t = Task()
        self.t("add one")
        self.t("add two")

        # Sometimes this test fails because the 1.uuid starts with N hex digits
        # such that those digits are all in the range [0-9], and therefore the
        # UUID looks like an integer.
        #
        # The only solution that comes to mind is to repeat self.t("add one")
        # until 1.uuid contains at least one [a-f] in the first N digits.

        code, self.uuid, err = self.t("_get 1.uuid")
        self.uuid = self.uuid.strip()

    def test_uuid_filter(self):
        """891: Test that a task is addressable using UUIDs of length 7 - 36"""
        for i in range(35,7,-1):
            code, out, err = self.t(self.uuid[0:i] + " list")
            self.assertIn("one", out)
            self.assertNotIn("two", out)

        # TODO This should fail because a 7-character UUID is not a UUID, but
        #      instead it blindly does nothing, and succeeds. Voodoo.
        #code, out, err = self.t(self.uuid[0:6] + " list")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
