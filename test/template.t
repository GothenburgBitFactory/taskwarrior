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
from datetime import datetime
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest import Taskd, ServerTestCase


# Test methods available:
#     self.assertEqual(a, b)
#     self.assertNotEqual(a, b)
#     self.assertTrue(x)
#     self.assertFalse(x)
#     self.assertIs(a, b)
#     self.assertIsNot(a, b)
#     self.assertIsNone(x)
#     self.assertIsNotNone(x)
#     self.assertIn(a, b)
#     self.assertNotIn(a, b)
#     self.assertIsInstance(a, b)
#     self.assertNotIsInstance(a, b)
#     self.assertRaises(e)
#     self.assertRegexpMatches(t, r)
#     self.assertNotRegexpMatches(t, r)
#     self.tap("")

class TestBugNumber(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        # Used to initialize objects that can be shared across tests
        # Also useful if none of the tests of the current TestCase performs
        # data alterations. See tw-285.t for an example

    def setUp(self):
        """Executed before each test in the class"""
        # Used to initialize objects that should be re-initialized or
        # re-created for each individual test
        self.t = Task()

    def test_version(self):
        """Copyright is current"""
        code, out, err = self.t("version")

        expected = "Copyright \(C\) \d{4} - %d" % (datetime.now().year,)
        self.assertRegexpMatches(out.decode("utf8"), expected)

        # TAP diagnostics on the bas
        self.tap("Yay TAP diagnostics")

    def test_faketime(self):
        """Running tests using libfaketime

           WARNING:
             faketime version 0.9.6 and later correctly propagates non-zero
             exit codes.  Please don't combine faketime tests and
             self.t.runError().

             https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=750721
        """
        self.t.faketime("-2y")

        command = ("add Testing")
        self.t(command)

        # Remove FAKETIME settings
        self.t.faketime()

        code, out, err = self.t("list")

        # Task should be 2 years old
        expected = "2.0y"
        self.assertIn(expected, out)

    def test_fail_other(self):
        """Nothing to do with Copyright"""
        self.assertEqual("I like to code", "I like\nto code\n")

    @unittest.skipIf(1 != 0, "This machine has sane logic")
    def test_skipped(self):
        """Test all logic of the world"""

    @unittest.expectedFailure
    def test_expected_failure(self):
        """Test something that fails and we know or expect that"""
        self.assertEqual(1, 0)

    def tearDown(self):
        """Executed after each test in the class"""

    @classmethod
    def tearDownClass(cls):
        """Executed once after all tests in the class"""


class TestHooksBugNumber(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.activate_hooks()

    def test_onmodify_custom(self):
        """Testing a custom made hook"""
        hookname = "on-modify-example-raw"

        content = """#!/usr/bin/env python
import sys
import json

original_task = sys.stdin.readline()
modified_task = sys.stdin.readline()

task = json.loads(modified_task)
task["description"] = "The hook did its magic"

sys.stdout.write(json.dumps(task, separators=(',', ':')) + '\\n')
sys.exit(0)
"""

        self.t.hooks.add(hookname, content)

        self.t("add Hello hooks")
        self.t("1 mod /Hello/Greetings/")
        code, out, err = self.t()
        self.assertIn("The hook did its magic", out)

        self.t.hooks[hookname].disable()
        self.assertFalse(self.t.hooks[hookname].is_active())

        self.t("1 mod /magic/thing/")
        code, out, err = self.t()
        self.assertIn("The hook did its thing", out)

    def test_onmodify_builtin_with_log(self):
        """Testing a builtin hook and keeping track of its input/output

        The builtin hook in found in test/test_hooks
        """
        hookname = "on-modify-for-template.py"
        self.t.hooks.add_default(hookname, log=True)

        self.t("add Hello hooks")
        self.t("1 mod /Hello/Greetings/")
        code, out, err = self.t()
        self.assertIn("This is an example modify hook", out)

        hook = self.t.hooks[hookname]
        logs = hook.get_logs()

        # Hook was called once
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        # Ensure output from hook is valid JSON
        # (according to python's JSON parser)
        hook.assertValidJSONOutput()

        # Checking which arguments were passed to the hook
        self.assertIn("/Hello/Greetings/", logs["calls"][0]["args"])

        # Some message output from the hook
        self.assertEqual(logs["output"]["msgs"][0],
                         "Hello from the template hook")

        # This is what taskwarrior received
        self.assertEqual(logs["output"]["json"][0]["description"],
                         "This is an example modify hook")

    def test_onmodify_bad_builtin_with_log(self):
        """Testing a builtin hook and keeping track of its input/output

        The builtin hook in found in test/test_hooks
        """
        hookname = "on-modify-for-template-badexit.py"
        self.t.hooks.add_default(hookname, log=True)

        self.t("add Hello hooks")
        self.t.runError("1 mod /Hello/Greetings/")
        code, out, err = self.t()
        self.assertNotIn("This is an example modify hook", out)

        hook = self.t.hooks[hookname]
        logs = hook.get_logs()

        # Hook was called once
        hook.assertTriggeredCount(1)
        hook.assertExitcode(1)

        # Some message output from the hook
        self.assertEqual(logs["output"]["msgs"][0],
                         "Hello from the template hook")

        # This is what taskwarrior would have used if hook finished cleanly
        self.assertEqual(logs["output"]["json"][0]["description"],
                         "This is an example modify hook")


class ServerTestBugNumber(ServerTestCase):
    @classmethod
    def setUpClass(cls):
        cls.taskd = Taskd()
        # This takes a while...
        cls.taskd.start()

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task(taskd=self.taskd)
        # Or if Task() is already available
        # self.t.bind_taskd_server(self.taskd)

    def test_server_sync(self):
        """Testing if client and server can speak to each other"""
        self.t("add Something to sync")
        self.t("sync")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
