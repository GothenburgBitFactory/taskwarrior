#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
from datetime import datetime
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase


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
        command = ("version",)

        code, out, err = self.t(command)

        expected = "Copyright \(C\) \d{4} - %d" % (datetime.now().year,)
        self.assertRegexpMatches(out.decode("utf8"), expected)

        # TAP diagnostics on the bas
        self.diag("Yay TAP diagnostics")

    def test_faketime(self):
        """Running tests using libfaketime"""
        self.t.faketime("-2y")

        command = ("add", "Testing")
        self.t(command)

        # Remove FAKETIME settings
        self.t.faketime()

        command = ("list",)
        code, out, err = self.t(command)

        # Task should be 2 years old
        expected = "2.0y"
        self.assertIn(expected, out)

    def test_fail_other(self):
        """Nothing to do with Copyright"""
        self.assertEqual("I like to code", "I like\nto code\n")

    @unittest.skipIf(1 != 0, "This machine has sane logic")
    def test_skipped(self):
        """Test all logic of the world"""

    def tearDown(self):
        """Executed after each test in the class"""

    @classmethod
    def tearDownClass(cls):
        """Executed once after all tests in the class"""


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
        self.t(("add", "Something to sync"))
        self.t(("sync",))


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
