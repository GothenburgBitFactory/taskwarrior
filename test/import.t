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
from basetest.utils import mkstemp


class TestImport(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("dateformat", "m/d/Y")

        self.data1 = """{"uuid":"00000000-0000-0000-0000-000000000000","description":"zero","project":"A","status":"pending","entry":"1234567889"}
{"uuid":"11111111-1111-1111-1111-111111111111","description":"one","project":"B","status":"pending","entry":"1234567889"}
{"uuid":"22222222-2222-2222-2222-222222222222","description":"two","status":"completed","entry":"1234524689","end":"1234524690"}
"""

        self.data2 = """{"uuid":"44444444-4444-4444-4444-444444444444","description":"three","status":"pending","entry":"1234567889"}
"""

    def assertData1(self):
        code, out, err = self.t(("list",))
        self.assertRegexpMatches(out, "1.+A.+zero")
        self.assertRegexpMatches(out, "2.+B.+one")
        self.assertNotIn("two", out)

        code, out, err = self.t(("completed",))
        self.assertNotIn("zero", out)
        self.assertNotIn("one", out)
        # complete has completion date as 1st column
        self.assertRegexpMatches(out, "2/13/2009.+two")

    def assertData2(self):
        code, out, err = self.t(("list",))
        self.assertRegexpMatches(out, "3.+three")

    def test_import_stdin(self):
        """Import from stdin"""
        code, out, err = self.t(("import", "-"), input=self.data1)
        self.assertIn("Imported 3 tasks", err)

        self.assertData1()

    def test_import_file(self):
        """Import from a file"""
        filename = mkstemp(self.data1)

        code, out, err = self.t(("import", filename))
        self.assertIn("Imported 3 tasks", err)

        self.assertData1()

    def test_double_import(self):
        """Multiple imports persist data"""
        code, out, err = self.t(("import", "-"), input=self.data1)
        self.assertIn("Imported 3 tasks", err)

        code, out, err = self.t(("import", "-"), input=self.data2)
        self.assertIn("Imported 1 tasks", err)

        self.assertData1()
        self.assertData2()

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
