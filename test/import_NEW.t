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


class TestImport(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("report.foo.description", "foo")
        self.t.config("report.foo.columns", "id,description,tags")
        self.t.config("report.foo.sort", "id+")

    def test_import_stdin(self):
        """When no files specified, import from STDIN"""
        json_data = """
        {"uuid":"00000000-0000-0000-0000-000000000000","description":"zero","project":"A","status":"pending","entry":"1234567889"}
        {"uuid":"11111111-1111-1111-1111-111111111111","description":"one","project":"B","status":"pending","entry":"1234567889"}
        {"uuid":"22222222-2222-2222-2222-222222222222","description":"two","status":"completed","entry":"1234524689","end":"1234524690"}"""  # borrowed from Perl import test
        code, out, err = self.t("import", input=json_data)
        self.assertRegexpMatches(err, "Imported 3 tasks.", "import all tasks")
        code, out, err = self.t("foo")
        self.assertRegexpMatches(out, "1\s+zero", "first task present")
        self.assertRegexpMatches(out, "2\s+one", "second task present")
        self.assertRegexpMatches(out, "-\s+two", "third task present")

    def test_import_update(self):
        """Update existing tasks"""
        json_data = """
        {"uuid":"00000000-0000-0000-0000-000000000000","description":"zero","project":"A","status":"pending","entry":"1234567889"}
        {"uuid":"11111111-1111-1111-1111-111111111111","description":"one","project":"B","status":"deleted","entry":"1234567889"}
        {"uuid":"22222222-2222-2222-2222-222222222222","description":"two","status":"completed","entry":"1234524689","end":"1234524690"}"""
        self.t("import", input=json_data)

        self.t("next")  # Run GC

        json_data = """
        {"uuid":"00000000-0000-0000-0000-000000000000","description":"zero","project":"C","status":"pending","entry":"1234567889"}
        {"uuid":"11111111-1111-1111-1111-111111111111","description":"one","project":"B","status":"pending","entry":"1234567889"}
        {"uuid":"22222222-2222-2222-2222-222222222222","description":"two","status":"pending","entry":"1234524689","end":"1234524690"}"""
        self.t("import", input=json_data)

        zero, one, two = sorted(self.t.export(), key=lambda t: t["uuid"])
        self.assertEqual(zero["status"], "pending", "status for 'zero' unchanged")
        self.assertEqual(zero["project"], "C", "project for 'zero' changed to 'C'")
        self.assertEqual(one["status"], "pending", "status for 'one' changed to pending")
        self.assertEqual(two["status"], "pending", "status for 'two' changed to pending")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
