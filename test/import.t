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
import json
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import mkstemp


class TestImport(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("dateformat", "m/d/Y")

        # Multiple tasks.
        self.data1 = """[
{"uuid":"a0000000-a000-a000-a000-a00000000000","description":"zero","project":"A","status":"pending","entry":"1234567889"},
{"uuid":"a1111111-a111-a111-a111-a11111111111","description":"one","project":"B","status":"pending","entry":"1234567889"},
{"uuid":"a2222222-a222-a222-a222-a22222222222","description":"two","status":"completed","entry":"1234524689","end":"1234524690"}
]
"""

        # Single task.
        self.data2 = """{"uuid":"44444444-4444-4444-4444-444444444444","description":"three","status":"pending","entry":"1234567889"}
"""

        # Free-form JSON.
        self.data3 = """

{
"uuid"
:
		"55555555-5555-5555-5555-555555555555"
,
"description"
:
"four"
,
    "status"
    :
    "pending"
    ,
"entry"
:
"1234567889"
}

"""

    def assertData1(self):
        code, out, err = self.t("list")
        self.assertRegexpMatches(out, "1.+A.+zero")
        self.assertRegexpMatches(out, "2.+B.+one")
        self.assertNotIn("two", out)

        code, out, err = self.t("completed")
        self.assertNotIn("zero", out)
        self.assertNotIn("one", out)
        # complete has completion date as 1st column
        self.assertRegexpMatches(out, "2/13/2009.+two")

    def assertData2(self):
        code, out, err = self.t("list")
        self.assertRegexpMatches(out, "3.+three")

    def assertData3(self):
        code, out, err = self.t("list")
        self.assertIn("four", out)

    def test_import_stdin(self):
        """Import from stdin"""
        code, out, err = self.t("import -", input=self.data1)
        self.assertIn("Imported 3 tasks", err)

        self.assertData1()

    def test_import_stdin_default(self):
        """Import from stdin is default"""
        code, out, err = self.t("import", input=self.data1)
        self.assertIn("Imported 3 tasks", err)

        self.assertData1()

    def test_import_file(self):
        """Import from a file"""
        filename = mkstemp(self.data1)

        code, out, err = self.t("import {0}".format(filename))
        self.assertIn("Imported 3 tasks", err)

        self.assertData1()

    def test_double_import(self):
        """Multiple imports persist data"""
        code, out, err = self.t("import -", input=self.data1)
        self.assertIn("Imported 3 tasks", err)

        code, out, err = self.t("import -", input=self.data2)
        self.assertIn("Imported 1 tasks", err)

        self.assertData1()
        self.assertData2()

    def test_freeform_import(self):
        """Import JSON with arbitrary formatting"""
        code, out, err = self.t("import -", input=self.data3)
        self.assertIn("Imported 1 tasks", err)

        self.assertData3()

    def test_import_update(self):
        """Update existing tasks"""
        self.t("import", input=self.data1)
        self.t("a1111111-a111-a111-a111-a11111111111 delete", input="y\n")
        self.t("next")  # Run GC

        _t = sorted(self.t.export(), key=lambda t: t["uuid"])
        _t[0]["project"] = "C"
        _t[1]["status"] = "pending"
        _t[2]["status"] = "pending"

        self.t("import", input="\n".join(json.dumps(t) for t in _t))

        _t = sorted(self.t.export(), key=lambda t: t["uuid"])
        self.assertEqual(_t[0]["status"], "pending")
        self.assertEqual(_t[0]["project"], "C")
        self.assertEqual(_t[1]["status"], "pending")
        self.assertEqual(_t[2]["status"], "pending")

    def test_import_python_json(self):
        """Python's default JSON formatting"""
        _t = json.loads(self.data1)
        code, out, err = self.t("import", input=json.dumps(_t))
        self.assertIn("Imported 3 tasks", err)
        self.assertData1()

    def test_import_no_newlines(self):
        """JSON array without newlines"""
        code, out, err = self.t("import", input=self.data1.replace("\n", ""))
        self.assertIn("Imported 3 tasks", err)
        self.assertData1()

    def test_import_newlines(self):
        """JSON array with newlines after each value"""
        _t = json.loads(self.data1)
        code, out, err = self.t("import", input=json.dumps(_t, indent=0))
        self.assertIn("Imported 3 tasks", err)
        self.assertData1()

    def test_import_newlines_whitespace(self):
        """JSON array with whitespace before and after names and values"""
        _data = """[
{ "uuid":"a0000000-a000-a000-a000-a00000000000" ,  "description"  :  "zero" ,"project":"A", "status":"pending","entry":"1234567889" } ,   
{ "uuid":"a1111111-a111-a111-a111-a11111111111","description":"one","project":"B","status":"pending","entry":"1234567889"},     {"uuid":"a2222222-a222-a222-a222-a22222222222","description":"two","status":"completed","entry":"1234524689","end":"1234524690" }
]"""
        code, out, err = self.t("import", input=_data)
        self.assertIn("Imported 3 tasks", err)
        self.assertData1()

    def test_import_old_depend(self):
        """One dependency used to be a plain string"""
        _data = """{"uuid":"a0000000-a000-a000-a000-a00000000000","depends":"a1111111-a111-a111-a111-a11111111111","description":"zero","project":"A","status":"pending","entry":"1234567889"}"""
        self.t("import", input=self.data1)
        self.t("import", input=_data)
        self.t.config("json.depends.array", "off")
        _t = self.t.export("a0000000-a000-a000-a000-a00000000000")[0]
        self.assertEqual(_t["depends"], "a1111111-a111-a111-a111-a11111111111")

    def test_import_old_depends(self):
        """Several dependencies used to be a comma seperated string"""
        _data = """{"uuid":"a0000000-a000-a000-a000-a00000000000","depends":"a1111111-a111-a111-a111-a11111111111,a2222222-a222-a222-a222-a22222222222","description":"zero","project":"A","status":"pending","entry":"1234567889"}"""
        self.t("import", input=self.data1)
        self.t("import", input=_data)
        _t = self.t.export("a0000000-a000-a000-a000-a00000000000")[0]
        self.assertIn("a1111111-a111-a111-a111-a11111111111", _t["depends"])
        self.assertIn("a2222222-a222-a222-a222-a22222222222", _t["depends"])

    def test_import_new_depend(self):
        """One dependency is a single array element"""
        self.t.config('json.depends.array', 'on')
        _data = """{"uuid":"a0000000-a000-a000-a000-a00000000000","depends":["a1111111-a111-a111-a111-a11111111111"],"description":"zero","project":"A","status":"pending","entry":"1234567889"}"""
        self.t("import", input=self.data1)
        self.t("import", input=_data)
        _t = self.t.export("a0000000-a000-a000-a000-a00000000000")[0]
        self.assertEqual(_t["depends"][0], "a1111111-a111-a111-a111-a11111111111")

    def test_import_new_depends(self):
        """Several dependencies are an array"""
        self.t.config('json.depends.array', 'on')
        _data = """{"uuid":"a0000000-a000-a000-a000-a00000000000","depends":["a1111111-a111-a111-a111-a11111111111","a2222222-a222-a222-a222-a22222222222"],"description":"zero","project":"A","status":"pending","entry":"1234567889"}"""
        self.t("import", input=self.data1)
        self.t("import", input=_data)
        _t = self.t.export("a0000000-a000-a000-a000-a00000000000")[0]

        for _uuid in ["a1111111-a111-a111-a111-a11111111111","a2222222-a222-a222-a222-a22222222222"]:
            self.assertTrue((_t["depends"][0] == _uuid) or (_t["depends"][1] == _uuid))

    def test_import_same_task_twice(self):
        """Test import same task twice"""
        _data = """{"uuid":"a1111111-a222-a333-a444-a55555555555","description":"data4"}"""
        self.t("import", input=_data)
        code, out1, err = self.t("export")
        self.t.faketime('+1s')
        self.t("import", input=_data)
        code, out2, err = self.t("export")
        self.assertEqual(out1, out2)


class TestImportExportRoundtrip(TestCase):
    def setUp(self):
        self.t1 = Task()
        self.t2 = Task()

        for client in (self.t1, self.t2):
            client.config("dateformat", "m/d/Y")
            client.config("verbose", "off")
            client.config("defaultwidth", "100")

    def _validate_data(self, client):
        code, out, err = client("_get 1.priority")
        self.assertEqual("H\n", out)
        code, out, err = client("_get 1.project")
        self.assertEqual("A\n", out)
        code, out, err = client("_get 1.description")
        self.assertEqual("one/1\n", out)
        code, out, err = client("_get 2.tags")
        self.assertEqual("tag1,tag2\n", out)
        code, out, err = client("_get 2.description")
        self.assertEqual("two\n", out)

    def test_import_export(self):
        """Test importing exported data"""
        self.t1("add priority:H project:A -- one/1")
        self.t1("add +tag1 +tag2 two")

        code, out1, err = self.t1("export")

        self.t2("import -", input=out1)
        code, out2, err = self.t2("export")

        self.assertEqual(out1, out2)

        self._validate_data(self.t1)
        self._validate_data(self.t2)


class TestImportValidate(TestCase):
    def setUp(self):
        self.t = Task()

    def test_import_empty_json(self):
        """Verify empty JSON is caught"""
        j = '{}'
        code, out, err = self.t.runError("import", input=j)
        self.assertIn("A task must have a description.", err)

    def test_import_invalid_uuid(self):
        """Verify invalid UUID is caught"""
        j = '{"uuid":"1", "description":"bad"}'
        code, out, err = self.t.runError("import", input=j)
        self.assertIn("Not a valid UUID", err)

    def test_import_invalid_uuid2(self):
        """Verify invalid UUID is caught, part two"""
        # UUID is the right length, but with s/-/0/.
        j = '{"uuid":"a1a1a1a10a1a10a1a10a1a10a1a1a1a1a1a1", "description":"bad"}'
        code, out, err = self.t.runError("import", input=j)
        self.assertIn("Not a valid UUID", err)

    def test_import_invalid_status(self):
        """Verify invalid status is caught"""
        j = '{"status":"foo", "description":"bad"}'
        code, out, err = self.t.runError("import", input=j)
        self.assertIn("The status 'foo' is not valid.", err)


class TestImportWithoutISO(TestCase):
    def setUp(self):
        self.t = Task()

    def test_import_with_iso_enabled(self):
        j = '{"uuid":"a2a2a2a2-a2a2-a2a2-a2a2-a2a2a2a2a2a2", "description":"one", "entry":"20151018T144200"}'
        self.t("import rc.date.iso=1", input=j)
        code, out, err = self.t("_get 1.entry")
        self.assertIn("2015-10-18T14:42:00\n", out)

    def test_import_with_iso_disabled(self):
        j = '{"uuid":"a2a2a2a2-a2a2-a2a2-a2a2-a2a2a2a2a2a2", "description":"one", "entry":"20151018T144200"}'
        self.t("import rc.date.iso=0", input=j)
        code, out, err = self.t("_get 1.entry")
        self.assertIn("2015-10-18T14:42:00\n", out)


class TestBug1441(TestCase):
    def setUp(self):
        self.t = Task()

    def test_import_filename(self):
        """1441: import fails if file doesn't exist"""
        code, out, err = self.t.runError("import xxx_doesnotexist")

        self.assertIn("File 'xxx_doesnotexist' not found.", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
